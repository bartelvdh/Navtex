
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "navtex_bytes_sm.h"
#define FILE_L 100


/* FROM THE IMO NAVTEX MANUAL
After between 60 h and 72 h, a message identification should automatically be erased from the store. If the number of received message identifications exceeds the capacity of the store, the oldest message identification should be erased.
*/

#define SECONDS_FOR_REMOVAL (60+72)/2*3600




void *file_purger(void *ptr)
{

	DIR *d;
 	struct dirent *dir;	
    	struct stat filestat;
	char filename[FILE_L];
        time_t mytime;
	//struct tm  *ptm;

        sleep(5);
		for(;;)
		{
		mytime = time ( NULL ); // Get local time in time_t
		//ptm = gmtime ( &mytime ); // Find out UTC time

		 d = opendir(FILE_DIR);
		 if (d) {
		     while ((dir = readdir(d)) != NULL) 
			 {
				if (dir->d_type == DT_REG)
				{
					strcpy(filename,FILE_DIR);
					strcat(filename,"/");
					strcat(filename,dir->d_name);

					if (!stat(filename, &filestat)) {
						if((mytime - filestat.st_ctime)>SECONDS_FOR_REMOVAL)
						{
							printf(" %s => TO DELETE\n",filename);
							remove(filename);
						}
					} 

				}
			  }
		  }

		  //repeat every hour
		  sleep(3600);

		}
}
