/* 
     This file is part of libmicrohttpd
     (C) 2007, 2008 Christian Grothoff (and other contributing authors)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
 * @file https_server_example.c
 * @brief a simple HTTPS file server using TLS.
 *
 * Usage :
 *
 *  'http_fileserver_example HTTP-PORT SECONDS-TO-RUN'
 *
 * The certificate & key are required by the server to operate,  Omitting the
 * path arguments will cause the server to use the hard coded example certificate & key.
 *
 * 'certtool' may be used to generate these if required.
 *
 * @author Sagie Amir
 */

//#include "platform.h"
#include <stdio.h>
#include <errno.h>
#include <microhttpd.h>
#include <sys/stat.h>
#include <gnutls/gnutls.h>
#include <gcrypt.h>
#include "../receiver/message_store.h"

#define BUF_SIZE 1024
#define MAX_URL_LEN 255

// TODO remove if unused
#define CAFILE "ca.pem"
#define CRLFILE "crl.pem"

#define EMPTY_PAGE "<html><head><title>File not found</title></head><body>File not found</body></html>"
#define TEST_PAGE "<html><head><title>test test test test test test</title></head><body>completed</body></html>"

static ssize_t
file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
  FILE *file = cls;

  (void) fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

static void
file_free_callback (void *cls)
{
  FILE *file = cls;
  fclose (file);
}

/* HTTP access handler call back */
//static int
enum MHD_Result 
http_ahc (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
	  size_t *upload_data_size, void **ptr)
{
  static int aptr;
  struct MHD_Response *response;
  int ret;
  FILE *file;
  struct stat buf;

  //printf("URL:%s len:%d\n",url,strlen(url));
  //printf("method: %s\n",method);
  if ( (0 != strcmp (method, MHD_HTTP_METHOD_GET))  &&  (0 != strcmp (method, MHD_HTTP_METHOD_POST)) && (0 != strcmp (method, "OPTIONS")) ) 
  {
    printf("unexpected method\n");
    return MHD_NO;              /* unexpected method */
  }
  if (&aptr != *ptr)
    {
      /* do never respond on first call */
      *ptr = &aptr;
      return MHD_YES;
    }
  *ptr = NULL;                  /* reset when done */
  if( strncmp(url,"/messagelist",11) == 0)
  {
      char *ml; 
      ml = get_message_list();
      response = MHD_create_response_from_buffer (strlen (ml),
						  (void *) ml,
						  MHD_RESPMEM_MUST_COPY);
      MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
      //MHD_add_response_header(response, "Referrer-Policy", "no-referrer");
      MHD_add_response_header(response, "Content-Type", "text/html; charset=UTF-8");
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      free_json_str(ml);
  }
  else if( strncmp(url,"/config",6) == 0)
  {
      char *ml; 
      ml = get_config();
      response = MHD_create_response_from_buffer (strlen (ml),
						  (void *) ml,
						  MHD_RESPMEM_MUST_COPY);
      MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
      //MHD_add_response_header(response, "Referrer-Policy", "no-referrer");
      MHD_add_response_header(response, "Content-Type", "text/html; charset=UTF-8");
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      free_json_str(ml);
  }
  else if( strncmp(url,"/saveconfig",11) == 0)
  {
      char* nconfig;
      char* ntag;
      char urlcpy[80];

	strcpy(urlcpy,url);

      char* token = strtok(urlcpy, "/");
 
      // Keep printing tokens while one of the
      // delimiters present in str[].
      if (token != NULL) {
          ntag = strtok(NULL, "/");
      }
      if (token != NULL) {
          nconfig = strtok(NULL, "/");
      }
      else 
      {
          return (-1);
      }
	


      update_config(ntag,nconfig);
      response = MHD_create_response_from_buffer (0,
                                                  (void *) NULL,
                                                  MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
      //MHD_add_response_header(response, "Referrer-Policy", "no-referrer");
      MHD_add_response_header(response, "Content-Type", "text/html; charset=UTF-8");
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
  }
  else if(strncmp(url,"/messagetext",12) == 0)
  {
      unsigned int m_id; 
      char *mt;

      sscanf(url,"/messagetext/%u",&m_id);
      mt = get_message_text(m_id);
      response = MHD_create_response_from_buffer (strlen (mt),
                                                  (void *) mt,
                                                  MHD_RESPMEM_MUST_COPY);
      MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
      MHD_add_response_header(response, "Content-Type", "text/html; charset=UTF-8");
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      free_json_str(mt);
  }
  else 
  {
    if ( (0 == stat (&url[1], &buf)) &&
	       (S_ISREG (buf.st_mode)) )
    {
	    file = fopen (&url[1], "rb");
    }
    else
    {
	    file = NULL;
    }


    if (file == NULL)
    {
      response = MHD_create_response_from_buffer (strlen (EMPTY_PAGE),
						  (void *) EMPTY_PAGE,
						  MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
      MHD_destroy_response (response);
    }
    else
    {
      response = MHD_create_response_from_callback (buf.st_size, 32 * 1024,     /* 32k PAGE_NOT_FOUND size */
                                                    &file_reader, file,
                                                    &file_free_callback);
      if (response == NULL)
	{
	  fclose (file);
	  return MHD_NO;
	}
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
    }
  }

  return ret;
}

int
main (int argc, char *const *argv)
{
  struct MHD_Daemon *TLS_daemon;
  int port_nr;

  if (chdir(WEB_ROOT) != 0)
    perror("chdir() to webroot failed");

  if (argc == 2)
  { 
    port_nr = atoi(argv[1]);
  }
  else
  { 
    port_nr = 8080;
  }
    
      /* TODO check if this is truly necessary -  disallow usage of the blocking /dev/random */
      /* gcry_control(GCRYCTL_ENABLE_QUICK_RANDOM, 0); */
      TLS_daemon =
        MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION,
				port_nr, 
				NULL, 
				NULL, 
				&http_ahc,
                          	NULL,
				MHD_OPTION_CONNECTION_TIMEOUT, 
				256,
                          	MHD_OPTION_END);

  if (TLS_daemon == NULL)
    {
      fprintf (stderr, "Error: failed to start TLS_daemon\n");
      return 1;
    }
  else
    {
      printf ("MHD daemon listening on port %d\n", port_nr);
    }


  for(;;)
  {
    sleep(62);
  }
  (void) getc (stdin);
  MHD_stop_daemon (TLS_daemon);
  return 0;
}
