// This is a simple client that uses the ACE Socket wrapper facades to
// establish a connection with the server and send/recv data.
//
// Original code for PA3 by Douglas C. Schmidt, modified/improved by
// Benjamin Peter
//
// Constraints:
//
// The client expects the thread id message to be send in a separate
// line and not to be bigger than BUFSIZ (which is typically 4-8k)
//
// The message of the client must only be line terminated (\r \n) at
// the end of the message.
// 
// The default port ACE_DEFAULT_SERVER_PORT (20002) is used.
//
// Compile for example using:
// g++ -I/usr/local/include -L/usr/local/lib -lACE -ldl -lrt simple-client-pa4.cpp -o simple-client-pa4
 
#include <algorithm>
#include <string>
#include <cstdio>
#include <cctype>
#include <assert.h>
#include "ace/OS.h"
#include "ace/SOCK_Connector.h"
#include "ace/Task.h"
#include "ace/Log_Msg.h"
 
// It's easy to change the type of connector by changing this typedef.
typedef ACE_SOCK_Connector IPC_CONNECTOR;
 
// Size of the buffer to send to the client.
static const int BUFFER_SIZE = BUFSIZ * 100;
 
// Number of iterations.
static const int MAX_ITERATIONS = 50;
 
// Number of seconds to wait for the results to come back from the server.
static const int TIMEOUT_SECS = 2;

// hexdump function from http://stahlworks.com/dev/index.php?tool=csc01.
static void 
hexdump (const void *pAddressIn, long lSize)
{
  char szBuf[100];
  long lIndent = 1;
  long lOutLen, lIndex, lIndex2, lOutLen2;
  long lRelPos;
  struct { const char *pData; unsigned long lSize; } buf;
  unsigned char *pTmp,ucTmp;
  const unsigned char *pAddress = (const unsigned char *)pAddressIn;
 
  buf.pData   = (const char *)pAddress;
  buf.lSize   = lSize;
 
  while (buf.lSize > 0)
    {
      pTmp     = (unsigned char *)buf.pData;
      lOutLen  = (int)buf.lSize;
      if (lOutLen > 16)
        lOutLen = 16;
 
      // create a 64-character formatted output line:
      sprintf(szBuf, " >                            "
              "                      "
              "    %08lX", pTmp-pAddress);
      lOutLen2 = lOutLen;
 
      for(lIndex = 1+lIndent, lIndex2 = 53-15+lIndent, lRelPos = 0;
          lOutLen2;
          lOutLen2--, lIndex += 2, lIndex2++
          )
        {
          ucTmp = *pTmp++;
 
          sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
          if(!isprint(ucTmp))  ucTmp = '.'; // nonprintable char
          szBuf[lIndex2] = ucTmp;
 
          if (!(++lRelPos & 3))     // extra blank after 4 bytes
            {  lIndex++; szBuf[lIndex+2] = ' '; }
        }
 
      if (!(lRelPos & 3)) lIndex--;
 
      szBuf[lIndex  ]   = '<';
      szBuf[lIndex+1]   = ' ';
 
      printf("%s\n", szBuf);
 
      buf.pData   += lOutLen;
      buf.lSize   -= lOutLen;
    }
}

// Dump the sent_buffer and received_buffer.
static void 
dump_buffers (const std::string &sent_buffer,
              ACE_Message_Block &received_buffer)
{
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("Original -------------------\n")));
  hexdump (sent_buffer.c_str (), (long) sent_buffer.size ());
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("Received -------------------\n")));
  hexdump (received_buffer.rd_ptr (), (long) sent_buffer.size ());
}
 
// Establish a connection to the server.
template <typename CONNECTOR> int
connect (CONNECTOR connector,
    typename CONNECTOR::PEER_ADDR addr,
    typename CONNECTOR::PEER_STREAM &peer)
{ 
  return connector.connect (peer, addr);
}
 
// Send a message to the server.
static int 
send_msg (IPC_CONNECTOR::PEER_STREAM &peer,
          const std::string &buffer) 
{
  // Wait up to TIMEOUT_SECS to send the buffer to the client.
  ACE_Time_Value timeout (TIMEOUT_SECS);
 
  // Perform a timed send to transmit all the data to the server.
  if (peer.send_n (buffer.c_str (),
                   buffer.length (),
                   &timeout) <= 0)
    ACE_ERROR_RETURN ((LM_ERROR,
                       "%s\n",
                       "send_n failed"),
                      -1);
  else
    return 0;
}
 
// Search for a line end character \n or \r or \r\n and return a
// pointer to it.
static char *
find_lineend (char *buf,
              size_t length) 
{
  char *nl = static_cast<char *> (memrchr (buf, '\n', length));
  if (nl != 0) 
    return nl;
 
  char *cr = static_cast<char *> (memrchr (buf, '\r', length));
  if (cr != 0) 
    {
      if (cr - buf < (ssize_t) length && *(cr + 1) == '\n') 
        return cr + 1;
      else 
        return cr;
    }
 
  return 0;
}
 
// Returns true if buffer contains a thread id.
static bool
contains_thread_id (char *buffer)
{
  errno = 0; // To distinguish success/failure after call.
  char *endptr;
  long t_id = ACE_OS::strtol (buffer, &endptr, 10);

  // Check if there was a numeric thread id returned.
  return !((errno == ERANGE && (t_id == LONG_MAX || t_id == LONG_MIN)) || (errno != 0 && t_id == 0))
    && endptr != buffer;
}

// Check to ensure the data from the server starts with a thread id
// followed by a newline.
static int 
read_and_verify_thread_id_msg (IPC_CONNECTOR::PEER_STREAM &peer,
                               ACE_Message_Block &message_block) 
{
  // Wait up to TIMEOUT_SECS to receive the buffer from the server.
  ACE_Time_Value thread_id_timeout (TIMEOUT_SECS);
 
  // Perform a timed receive to obtain the thread id from the client
  ssize_t bytes_read = peer.recv (message_block.wr_ptr (),
                                  BUFSIZ,
                                  &thread_id_timeout);
  switch (bytes_read)
    {
    case -1:
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("%p\n"),
                         ACE_TEXT ("recv of thread id failed")),
                        -1);
    case 0:
      // FALLTHRU
    default:
      // Try to find a newline character in the list and store a
      // pointer to the end. If non is found 0 is returned.
      char *line_end = find_lineend (message_block.rd_ptr (),
                                     bytes_read);
      assert (line_end != 0);

      // Replace the newline with a NUL.
      *line_end = '\0';

      assert (contains_thread_id (message_block.rd_ptr ()));

      // Write the received thread id to stdout.
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("(%P|%t) server echoed back thread id %*s\n"),
                  line_end - message_block.rd_ptr (),
                  message_block.rd_ptr ()));
 
      // Advance the message block pointers so rd_ptr() points to
      // after the thread id message.  Next we are going to receive
      // the actual payload into wr_ptr().
      message_block.rd_ptr (line_end + 1);
      message_block.wr_ptr (bytes_read);
 
      return 0;
    }
}
 
static int 
read_and_verify_msg (IPC_CONNECTOR::PEER_STREAM &peer,
                     ACE_Message_Block &message_block,
                     const std::string &buffer) 
{
  size_t bytes_read;
  ACE_Time_Value timeout (TIMEOUT_SECS);
 
  // Dumping the message block here can help if you have a problem
  // with the thread id message sending.
  //
  // message_block.dump();
 
  // Perform a timed receive to obtain the all data echoed back
  // from the server.
  switch (peer.recv_n (message_block.wr_ptr (),
                       buffer.length () - message_block.length (),
                       0,
                       &timeout,
                       &bytes_read))
    {
    case -1:
      ACE_ERROR_RETURN ((LM_ERROR,
                         "%p, bytes_read (%d)\n",
                         "recv_n failed",
                         bytes_read),
                        -1);
    case 0:
      // FALLTHRU
    default:
      
      // Advance the wr_ptr() past the bytes read.
      message_block.wr_ptr (bytes_read);
 
      // Make sure that what was sent is what was echoed back.
      assert (message_block.length() == buffer.length ());

      if (!std::equal (buffer.begin (),
                       buffer.end (),
                       message_block.rd_ptr())) 
        dump_buffers (buffer, message_block);
 
      assert (std::equal (buffer.begin (),
                          buffer.end (),
                          message_block.rd_ptr ()));
 
      // If we made it this far we're in good shape!
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("(%P|%t) server echoed back the data correctly\n")));
      return 0;
    }
}
 
// Receive the thread id and buffer echoed by the server.

static int 
receive (IPC_CONNECTOR::PEER_STREAM &peer,
         const std::string &sent_buffer)
{
  // Create a message block that can hold the message we are expecting
  // plus an additional amount for the thread id message upfront.
  ACE_Message_Block received_buffer (BUFSIZ + BUFFER_SIZE);
    
  if (read_and_verify_thread_id_msg (peer, received_buffer) == -1)
    return -1;
  else if (read_and_verify_msg (peer, received_buffer, sent_buffer) == -1) 
    return -1;
  else
    return 0;
}

// Send data to the server and check to ensure it's echoed back
// properly.
 
static int 
send_and_receive_n (IPC_CONNECTOR::PEER_STREAM &peer,
                    const std::string &sent_buffer,
                    size_t iterations) 
{
  for (size_t i = 0; i < iterations; ++i) 
    if (send_msg (peer, sent_buffer) == -1)
      return -1;
    else if (receive (peer, sent_buffer) == -1)
      return -1;

  return 0;
}
 
int 
main (int argc, char *argv[]) 
{ 
  // Parse command-line parameters (if any are given).
  std::string server_hostname = argc > 1 ? argv[1] : "127.0.0.1";
  size_t iterations = argc > 2 ? ACE_OS::atoi (argv[2]) : MAX_ITERATIONS;
 
  IPC_CONNECTOR connector;
  // Note the ues of C++ traits to generalize the type of IPC mechanism used.
  IPC_CONNECTOR::PEER_ADDR addr (50000, //ACE_DEFAULT_SERVER_PORT,
                                 server_hostname.c_str());
  IPC_CONNECTOR::PEER_STREAM peer;
 
  std::string buffer;
 
  // Initialize the buffer with random data.
  ACE_OS::srand (ACE_OS::time (0));
  std::generate_n (std::back_inserter (buffer),
                   BUFFER_SIZE - 2,
                   rand);
  // Replace newline characters since this would confuse our server.
  std::replace (buffer.begin (), buffer.end (), '\r', 'X');
  std::replace (buffer.begin (), buffer.end (), '\n', 'Y');
  std::replace (buffer.begin (), buffer.end (), (char) 0x04, 'Z'); // Ctrl+d

  // Put a Windows-style newline at the end.
  std::back_inserter (buffer) = '\r';
  std::back_inserter (buffer) = '\n';
 
  // Establish the connection and send/recv the data to/from the
  // server.
  if (connect (connector, addr, peer) == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%P|%t) connect failed\n")),
                      -1);
 
  return send_and_receive_n (peer,
                             buffer,
                             iterations);
}
