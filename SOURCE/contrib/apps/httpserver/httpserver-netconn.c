
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/ip4_addr.h"
#include "lwip/apps/httpd.h"

#include <string.h>

#include "httpserver-netconn.h"

#if LWIP_NETCONN

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG         LWIP_DBG_OFF
#endif

static const char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
static const char http_index_html[] = "<html><head><title>Test</title><style>.clickme{background-color: #EEEEEE;padding: 8px 20px;text-decoration:none;font-weight:bold;border-radius:5px;cursor:pointer;}.danger{background-color:#FF0040;color: #FFFFFF;}.danger:hover{background-color:#EB003B;color: #FFFFFF;}</style></head><body style=\"background-color: #dfdfe2;\"><img src=\"https://static.wikia.nocookie.net/lwip/images/b/bc/Wiki.png/\"><h1 style=\"font-family:'Courier New'\">lwIP Test Application</h1><p style=\"background-color: #d0d0d5; font-family:'Courier New'; font-size: 20px;\">The following link goes to the /identification endpoint to test GET-Request functionality of said endpoint. The Enpoint should return a json object containint Information. Among other things it should contain the IP-Adress of the lwIP-host application</p><a href=\"/identification\" class=\"clickme danger\">Get Test</a></body></html>";
static const char http_html_hdr2[] = "HTTP/1.1 200 OK\r\nContent-type: application/json\r\n\r\n";

/** Serve one HTTP connection accepted in the http thread */
static void
http_server_netconn_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  
  /* Read the data from the port, blocking if nothing yet there. 
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);
  
  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    
    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' &&
        buf[5] == ' ') {
      
      /* Send the HTML header 
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */
      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
      
      /* Send our HTML page */
      netconn_write(conn, http_index_html, sizeof(http_index_html)-1, NETCONN_NOCOPY);
    }
    if (buflen>=7 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' &&
        buf[5]=='i' &&
        buf[6]=='d' &&
        buf[7]=='e' &&
        buf[8]=='n' &&
        buf[9]=='t' &&
        buf[10]=='i' &&
        buf[11]=='f' &&
        buf[12]=='i' &&
        buf[13]=='c' &&
        buf[14]=='a' &&
        buf[15]=='t' &&
        buf[16]=='i' &&
        buf[17]=='o' &&
        buf[18]=='n') {
 
        int i=0;
        int j=1;
        char ip[18]="";
        while(buf[i]) {
            if(buf[i]=='\n'&&buf[i+1]=='H') {
                printf("%c",buf[i + 1]);
                i+=7;
                while(buf[i]!='\r') {
                    ip[0]='"';
                    ip[j]=buf[i];
                    i++;
                    j++;
                }
                ip[j]='"';
                break;
            };
            i++;
        }

        char startBlock[] = "{ \"DeviceClass\": \"Windows PC\", \"Manufacturer\": \"lwIP - A Lightweight TCP/IP stack\"";
        char ipBlock[] = ", \"IP-Adress\": ";
        char secondBlock[] = ",\"Model\": null, \"ProductCode\": null, \"HardwareRevision\" : 1, \"SoftwareRevision\" : \"2.1.0\", \"SerialNumber\" : null, \"ProductInstanceUri\" : \"https://savannah.nongnu.org/projects/lwip/\", \"applicationSpecificTag\" : null, \"geolocation\": null, \"sysUpTime\" : null";
        char endBlock[] = " }";
       
        char *http_index_html2;
        int size=sizeof(startBlock)-1+sizeof(ipBlock)-1+j+1+sizeof(secondBlock)-1+sizeof(endBlock);
        http_index_html2 = (char*)malloc(size);
        strcpy(http_index_html2,startBlock);
        strcat(http_index_html2,ipBlock);
        strcat(http_index_html2,ip);
        strcat(http_index_html2,secondBlock);
        strcat(http_index_html2,endBlock);
        /* Send the HTML header
               * subtract 1 from the size, since we dont send the \0 in the string
               * NETCONN_NOCOPY: our data is const static, so no need to copy it
         */
        netconn_write(conn, http_html_hdr2, sizeof(http_html_hdr2) - 1, NETCONN_NOCOPY);

        /* Send our HTML page */
        netconn_write(conn, http_index_html2, size - 1, NETCONN_NOCOPY);
    }
  
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);
  
  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

/** The main function, never returns! */
static void
http_server_netconn_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);
  
  /* Create a new TCP connection handle */
  /* Bind to port 80 (HTTP) with default IP address */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 80);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 80);
#endif /* LWIP_IPV6 */
  LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);
  
  /* Put the connection into LISTEN state */
  netconn_listen(conn);
  
  do {
    err = netconn_accept(conn, &newconn);
    if (err == ERR_OK) {
      http_server_netconn_serve(newconn);
      netconn_delete(newconn);
    }
  } while(err == ERR_OK);
  LWIP_DEBUGF(HTTPD_DEBUG,
    ("http_server_netconn_thread: netconn_accept received error %d, shutting down",
    err));
  netconn_close(conn);
  netconn_delete(conn);
}

/** Initialize the HTTP server (start its thread) */
void
http_server_netconn_init(void)
{
  sys_thread_new("http_server_netconn", http_server_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}

#endif /* LWIP_NETCONN*/
