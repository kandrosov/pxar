#include <libusb.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <time.h> // needed for usleep function

#include "USBInterface.h"

// needed for threaded readout of FTDI
#include <pthread.h> 
#include <semaphore.h>

static struct ftdi_context ftdic;

// the read buffer needs to be accessable outside of our USB class
#define BUFSIZE 0x200000
static pthread_t readerthread;
static sem_t buf_data, buf_space;
static unsigned char read_buffer[BUFSIZE];
static int32_t head, tail; // read buffer is used as ring buffer

// cleanup is threaded to include a timeout on the calls to the device that sometimes hang
pthread_mutex_t cleanup_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t usbclose_thread, usbdeinit_thread;
volatile bool usbclose_done, usbdeinit_done;

const int32_t productID_FT232H = 0x6014; // new testboard FTDI chip product id (FT232H)
const int32_t productID_OLD = 0x6001; //  single channel devices (R Chips) used in older test boards
const int32_t vendorID = 0x0403; // Future Technology Devices International, Ltd

using namespace std;

static void add_to_buf (unsigned char c) {
    int32_t nh;

    sem_wait (&buf_space);
    if (head == (BUFSIZE -1)) nh = 0;
    else                      nh = head + 1;

    if (nh == tail) {
        fprintf (stderr, "USBInterface (libftdi) FATAL: overflow of circular buffer in threaded read routine. Sorry -- will exit now.\n");
        exit (1);
    }
    read_buffer[head] = c;
    head = nh;
    sem_post (&buf_data);
}

static void *reader (void *arg) {
  // there is no non-blocking read command implemented in libftdi ->
  // therefore we use multithreading and a static buffer to emulate
  // non-blocking calls
    struct ftdi_context *handle = (struct ftdi_context *)(arg);
    unsigned char buf[0x1000];
    int32_t br, i;

    while (1) {
      usleep(100); // wait 0.1 ms
      pthread_testcancel();
      br = ftdi_read_data (handle, buf, sizeof(buf));
      pthread_testcancel();
      if (br< 0){
	std::cout << " ERROR during USB read polling: error code from libusb_bulk_transfer(): " << br << std::endl;
      }
      if (br > 0){
	for (i=0; i<br; i++){
	  add_to_buf (buf[i]);
	}
      }
    }
    return NULL;
}

static void *usbclose (void *arg) {
  // on some circumstances, the ftdi_usb_close() call hangs;
  // this is a workaround to implement a timeout
    struct ftdi_context *handle = (struct ftdi_context *)(arg);
    ftdi_usb_close(handle);
    pthread_mutex_lock(&cleanup_mutex); usbclose_done = true; pthread_mutex_unlock(&cleanup_mutex);
    return NULL;
}

static void *usbdeinit (void *arg) {
  // on some circumstances, the ftdi_deinit() call hangs;
  // this is a workaround to implement a timeout
    struct ftdi_context *handle = (struct ftdi_context *)(arg);
    ftdi_deinit(handle);
    pthread_mutex_lock(&cleanup_mutex); usbdeinit_done = true; pthread_mutex_unlock(&cleanup_mutex);
    return NULL;
}

uint32_t FindAllUSB(struct ftdi_device_list ** devlist){
  int status;
  uint32_t nDevices = 0;
  struct ftdi_device_list *  	devlist_atb;

  // For backward (driver) compatibility:
  // Old libfti versions do not allow wildcards for vendorID and productID.
  // This first checks explicitly for DTB boards, then for ATB ones and merges the device lists

  // DTB
  status =  ftdi_usb_find_all(&ftdic, devlist,vendorID,productID_FT232H);
  if( status < 0) {
    return status;
  }
  if ( status > 0 ){
    nDevices+=status;
  }

  // ATB
  status =  ftdi_usb_find_all(&ftdic, &devlist_atb,vendorID,productID_OLD);
  if( status < 0) {
    return status;
  }
  if ( status > 0 ){
    // merge device lists
    struct ftdi_device_list **curdev;
    curdev = devlist;
    for (uint32_t i = 0; i<nDevices;i++) curdev=&(*curdev)->next; // go to end of DTB device list
    (*curdev)->dev = devlist_atb->dev; // put pointer to last devlist entry to first ATB entry
    nDevices+=status; // add number of ATB devices to total number
  }
  return nDevices;
}



CUSB::CUSB(){
      m_posR = m_sizeR = m_posW = 0;
      m_timeout = 150000; // maximum time to wait for read call in ms
      isUSB_open = false;
      ftdiStatus = 0;
      enumPos = enumCount = 0;
      ftdiStatus = ftdi_init(&ftdic);
      if ( ftdiStatus < 0)
	{
	  cout <<  "USBInterface constructor: ftdi_init failed" << endl;
	  exit(1);
	}
}

CUSB::~CUSB(){ 
  if (isUSB_open) Close(); 
  pthread_mutex_lock(&cleanup_mutex); usbdeinit_done = false; pthread_mutex_unlock(&cleanup_mutex);
  // create cleanup thread to allow timeout freeing the USB handle (might hang sometimes)
  pthread_create (&usbdeinit_thread, NULL, usbdeinit, &ftdic);
  bool done = false;
  for (int time = 0; time<1000;time++){
    usleep(1000); // wait 1ms
    // check status and break if usbdevice is closed
    pthread_mutex_lock(&cleanup_mutex); 
    if (usbdeinit_done) {
      // cout << " DEBUG: successfully free'd usb handle connection " << endl;
      done = true;    }
    pthread_mutex_unlock(&cleanup_mutex);
    if (done) break;  }
  //if (!done) cout << " WARNING: freeing the USB handle timed out! " << endl;
}

const char* CUSB::GetErrorMsg()
{
  return ftdi_get_error_string(&ftdic);
}


bool CUSB::EnumFirst(uint32_t &nDevices)
{
  struct ftdi_device_list *  	devlist;

  ftdiStatus = FindAllUSB(&devlist);
  if( ftdiStatus <= 0) {
    nDevices = enumCount = enumPos = 0;
    return false;
  }
  ftdi_list_free(&devlist);
  enumCount = ftdiStatus;
  nDevices = ftdiStatus;
  enumPos = 0;
  return true;
}


bool CUSB::EnumNext(char name[])
{
  if( isUSB_open) { 
    std::cout << " Warning: Trying to call USBInterface::EnumNext() while other USB device still open" << std::endl;
    return false; 
  }
  struct ftdi_device_list *  	devlist;
  ftdiStatus =  FindAllUSB(&devlist);
  if( ftdiStatus <= 0) {
    enumCount = enumPos = 0;
    return false;
  }
  enumCount = ftdiStatus;
  if( enumPos >= enumCount) return false;

  // go to the position of the enumPos pointer
  for (uint32_t i=0; i<enumPos; i++) devlist = devlist->next;
  
  char manufacturer[128], description[128], serial[128];

  if ((ftdiStatus = ftdi_usb_get_strings(&ftdic,devlist->dev, manufacturer, 128, description, 128, serial, 128)) < 0)
    {
      std::cout << " USBInterface::EnumNext(): Error polling USB device number " << enumPos << std::endl;
      return EXIT_FAILURE;
    }
  strcpy(name,serial); // return device string information for a single device
  ftdi_list_free(&devlist);
  enumPos++;
  return true;
}


bool CUSB::Enum(char name[], uint32_t pos)
{
  if( isUSB_open) { 
    std::cout << " Warning: Trying to call USBInterface::Enum() while other USB device still open" << std::endl;
    return false; 
  }

  struct ftdi_device_list *  	devlist;
  ftdiStatus =  FindAllUSB(&devlist);
  if( ftdiStatus <= 0) {
    enumCount = enumPos = 0;
    return false;
  }
  enumCount = ftdiStatus;
  if( pos > enumCount) return false;

  // go to the position of the pos argument
  for (uint32_t i=0; i<pos; i++) devlist = devlist->next;
  
  char manufacturer[128], description[128], serial[128];
  if ((ftdiStatus = ftdi_usb_get_strings(&ftdic,devlist->dev, manufacturer, 128, description, 128, serial, 128)) < 0)
    {
      std::cout << " USBInterface::EnumNext(): Error polling USB device number " << pos << std::endl;
      return EXIT_FAILURE;
    }
  strcpy(name,serial); // return device string information for a single device
  ftdi_list_free(&devlist);
  enumPos = pos;
  return true;
}



bool CUSB::Open(char serialNumber[])
{
  if( isUSB_open) { 
    std::cout << " Warning: Trying to open new USB device while other device still open" << std::endl;
    return false; 
  }
  
  std::cout << " USBInterface::Open(): searching for device with serial number: '" << serialNumber << "'" << std::endl;

  // reset buffer index positions
  m_posR = m_sizeR = m_posW = 0;

  // open list of usb devices with the expected vendor and product ids
  struct ftdi_device_list *  	devlist;
  ftdiStatus =  FindAllUSB(&devlist);
  
  if( ftdiStatus <= 0) {
    std::cout << " USBInterface::Open(): Error searching attached USB devices! ftdiStatus: " << ftdiStatus << std::endl;
    return EXIT_FAILURE;
  }

  // loop over all devices to check for matching serial number
  int32_t ndevices = ftdiStatus;
  for (int32_t i=0; i<ndevices; i++) {
    char manufacturer[128], description[128], serial[128];
    if ((ftdiStatus = 
	 ftdi_usb_get_strings(&ftdic,devlist->dev, manufacturer, 
			      128, description, 128, serial, 128)) < 0){
      std::cout << " USBInterface::Open(): Error polling USB device number " << i << std::endl;
      devlist = devlist->next;
      continue;
    }    
    if (strcmp(serialNumber,serial)!=0 && strcmp(serialNumber,"*")!=0){
      // not found, next device
      std::cout << " USBInterface::Open(): found non-matching device with serial number: '" << serial << "'" << std::endl;

      devlist = devlist->next;
    } else {
      // found the device
      std::cout << " USBInterface::Open(): found device with serial " << serial << std::endl;
      // now open it
      ftdiStatus = ftdi_usb_open_dev(&ftdic, devlist->dev);
      if( ftdiStatus < 0) {
	/* maybe the ftdi_sio and usbserial kernel modules are attached to the device */
	/* try to detach them using the libusb library directly */
	std::cout << " Warning: FTDI returned status code " << ftdiStatus << ", will try to detach ftdi_sio and usbserial kernel modules " << std::endl;
	libusb_device_handle *handle;
	/* open the device */
	int32_t ok = libusb_open((libusb_device*)devlist->dev, &handle);
	if( ok != 0){
	  std::cout << " Warning: libusb returned status code " << ok << ", could not get USB device handle " << std::endl;
	  ftdi_list_free(&devlist);
	  return EXIT_FAILURE;
	}
	
	/* Detach the kernel module from the device */
        ok = libusb_detach_kernel_driver(handle, 0);
        if( ok == 0){
	  std::cout << " Detached kernel driver from selected testboard. " << std::endl;
        } else {
          std::cout << "Unable to detach kernel driver from selected testboard." << std::endl;
	}
	libusb_close(handle);

	// now open it again
	ftdiStatus = ftdi_usb_open_dev(&ftdic, devlist->dev);
	if( ftdiStatus < 0) {
	  std::cout << " Warning: FTDI returned status code " << ftdiStatus << ", will try to detach ftdi_sio and usbserial kernel modules " << std::endl;
	  ftdi_list_free(&devlist);
	  return EXIT_FAILURE;
	}
      }
      isUSB_open = true;
      std::cout << " FTDI successfully opened connection to device " << std::endl;

    } // strcmp serial
  } // device loop
  
  ftdi_list_free(&devlist);
  if (!isUSB_open){
    std::cout << " Device with serial '" << serialNumber <<  "' not found! :-( " << std::endl;
    return EXIT_FAILURE;
  }

  //std::cout << " resetting mode for FTDI chip " << std::endl;
  int32_t status =  ftdi_set_bitmode(&ftdic, 0xFF, 0x40);
  if (status < 0){
    std::cout << " ERROR issuing reset: return code " << status << std::endl;
  }
  usleep(10000); // wait 10 ms
  //std::cout << " setting bit mode for FTDI chip " << std::endl;
  status =  ftdi_set_bitmode(&ftdic, 0xFF, 0x40);
  if (status < 0){
    std::cout << " ERROR setting bit mode: return code " << status << std::endl;
  }

  // init threads for client-side data buffering
  sem_init (&buf_data, 0, 0);
  sem_init (&buf_space, 0, BUFSIZE);
  pthread_create (&readerthread, NULL, reader, &ftdic);

  return true;
}


void CUSB::Close(){
  if( !isUSB_open) return;
  pthread_cancel(readerthread);
  usleep(10000);
  // join reader thread and clean up semaphores
  pthread_join(readerthread, NULL);
  sem_destroy (&buf_data);
  sem_destroy (&buf_space);
  usleep(10000);
  // set the flag (lock mutex first)
  pthread_mutex_lock(&cleanup_mutex); usbclose_done = false; pthread_mutex_unlock(&cleanup_mutex);
  // create cleanup thread to allow timeout on call to device (might hang)
  pthread_create (&usbclose_thread, NULL, usbclose, &ftdic);
  bool done = false;
  for (int time = 0; time<1000;time++){
    usleep(1000); // wait 1ms
    // check status and break if usbdevice is closed
    pthread_mutex_lock(&cleanup_mutex);  // lock mutex
    if (usbclose_done) {
      //cout << " DEBUG: successfully closed usb connection " << endl;
      done = true; }
    pthread_mutex_unlock(&cleanup_mutex); // unlock mutex
    if (done) break;}
  //if (!done) cout << " WARNING: closing the USB connection timed out! " << endl;
  isUSB_open = 0;
}

void CUSB::WriteCommand(unsigned char x){
  const unsigned char CommandChar = ESC_EXTENDED; 
  Write(sizeof(char), &CommandChar); // ESC_EXTENDED 
  Write(sizeof(char),&x);
}

void CUSB::Write(uint32_t bytesToWrite, const void *buffer)
{ 
    if (!isUSB_open) throw CRpcError(CRpcError::WRITE_ERROR);
  uint32_t k=0;
  for( k=0; k < bytesToWrite; k++ ) {
    if( m_posW >= USBWRITEBUFFERSIZE) {Flush();}
    m_bufferW[m_posW++] = ((unsigned char*)buffer)[k];
  }
  return;
}


void CUSB::Flush()
{ 
  int32_t bytesToWrite = m_posW;
  m_posW = 0;

  if (!isUSB_open) throw CRpcError(CRpcError::WRITE_ERROR);

  if( !bytesToWrite) return;

  ftdiStatus = ftdi_write_data(&ftdic, m_bufferW, bytesToWrite);

  if( ftdiStatus < 0)  throw CRpcError(CRpcError::WRITE_ERROR);
  if( ftdiStatus != bytesToWrite) { 
    std::cout<< " Warning: USBInterface: mismatch of bytes sent to USB chip and bytes written! " << std::endl;
    throw CRpcError(CRpcError::WRITE_ERROR);
  }
}

bool CUSB::FillBuffer(uint32_t /*minBytesToRead*/)
{
  cout << " USBInterface: FillBuffer() called but this function is not implemted anymore for libftdi " << endl;
  return true;
}


void CUSB::Read(uint32_t bytesToRead, void *buffer, uint32_t &bytesRead)
{
   if (!isUSB_open) throw CRpcError(CRpcError::READ_ERROR);
 
   // Copy over data from the circular buffer
    uint32_t i;
    uint32_t timewasted = 0; // time in ms wasted in this routine

      for (i=0; i<bytesToRead; i++) {
	bool bufferready = true;
	if (tail == head) bufferready = false;
	if (!bufferready){
	  while (!bufferready && timewasted<m_timeout){
	    if (timewasted==(m_timeout/10)) cout<< "USBInterface: Read(): data not ready after " << timewasted << "ms yet! Will wait for up to " << m_timeout << "ms"<< flush;
	    if (timewasted>m_timeout && timewasted%100==0 ) cout << "." << flush;
	    usleep(1000); //wait 1 ms
	    timewasted++;
	    if (tail != head) bufferready = true;	    
	  }
	  // if timeout message was printed before show the conclusion now
	  if (timewasted >= (m_timeout/10)){
	    if (bufferready) cout << "..done!" << endl;
	    else cout << "..failed! :(    .. maybe adjust timeout setting (method SetTimeout(int)) for this call?" <<endl;
	  }
	}
	if (bufferready){
	  sem_wait (&buf_data);	
	  ((unsigned char*)buffer)[i] = read_buffer[tail];
	  if (tail == (BUFSIZE -1)) tail = 0;
	  else                      tail++;
	  sem_post (&buf_space);
	} 
	else // buffer was not ready and reading it timed out so we stop attempting it now
	  {
	    bytesRead = i;
	    throw CRpcError(CRpcError::READ_TIMEOUT);
	    break;
	  }
      }
      bytesRead = i;
}

//----------------------------------------------------------------------
void CUSB::Clear()
{
  if( !isUSB_open) return;

  ftdiStatus = ftdi_usb_purge_buffers(&ftdic);

  // drain our buffer.
  while (head != tail) {
    unsigned char t;
    uint32_t n;
    Read(1, &t, n);
  }

  m_posR = m_sizeR = 0;
  m_posW = 0;
}

//----------------------------------------------------------------------
bool CUSB::Show()
{
  cout << " USB status: " << endl;
  if( !isUSB_open) {
    cout << "  - USB connection not open " << endl;
    return false;
  }
  cout << "  - max timeout for read calls set to " << m_timeout << "ms" << endl;

  unsigned char latency;
  if (ftdi_get_latency_timer(&ftdic,&latency)==0)  cout << "  - FTDI latency timer set to " << (int) latency << endl;
  cout << "  - data waiting in local read buffer: " << !(tail == head) << endl;
  

  
  return true;
}

//----------------------------------------------------------------------
int32_t CUSB::GetQueue()
{
  cout << " USBInterface: GetQueue() called but this function is not implemted for libftdi " << endl;
  return 0;
}

bool CUSB::WaitForFilledQueue(int32_t /*pSize*/,int32_t /*pMaxWait*/)
{
  // this function has no purpose when using libftdi: we implement our own read buffer and poll 
  if( !isUSB_open ) return false;
  return true;
}

//----------------------------------------------------------------------
void CUSB::Read_String(char *s, uint16_t maxlength)
{
  char ch = 0;
  uint16_t i=0;
  do {
    Read_CHAR(ch);
    if( i<maxlength) { s[i] = ch; i++; }
  }
  while (ch != 0);
  if( i >= maxlength) s[maxlength-1] = 0;
}


void CUSB::Write_String(const char *s)
{
  do {
    Write_CHAR(*s);
    s++;
  }
  while (*s != 0);
}
