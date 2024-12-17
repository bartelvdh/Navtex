#define _XOPEN_SOURCE   
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <sqlite3.h>
#include <time.h>
#include "message_store.h"

#define TS_SIZE 20

#define MESSAGE_PURGE_AGE 60*60*72  			//72 hours in seconds
#define MESSAGE_WEAHTER_FCST_AGE_LIMIT 60*60*28  	//28 hours in seconds mark weather fcsts as old when reaching this age



time_t my_timegm(struct tm *tm) {
    time_t t = mktime(tm);       // mktime assumes the input is in local time
    return t - timezone;         // Adjust for timezone offset to get UTC time
} 

void get_current_timestamp(char *cts)
{
   struct tm timeinfo;
   time_t timestamp;

   timestamp = time(NULL);
   gmtime_r(&timestamp, &timeinfo); 
   strftime(cts, TS_SIZE, "%Y-%m-%d %H:%M", &timeinfo);
}


// Function to calculate the difference in seconds between timestamp and current time
long calculate_time_difference(char *timestamp) {
    struct tm tm;
    time_t given_time, current_time;

    // Parse the input timestamp string into struct tm
    memset(&tm, 0, sizeof(struct tm)); // Initialize the structure to zero
    strptime(timestamp, "%Y-%m-%d %H:%M", &tm);

    // Convert struct tm to time_t (seconds since epoch)
    given_time = my_timegm(&tm);

    // Get the current time in UTC
    current_time = time(NULL);

    // Calculate and return the difference in seconds
    return difftime(current_time, given_time);
}

int add_message(char *bbbb, char *message, int freq) {
        char timestamp_str[TS_SIZE];

        sqlite3 *db;
        sqlite3_stmt *stmt;


        get_current_timestamp(timestamp_str);
         
        sqlite3_open("navtex.db", &db);
        if (db == NULL)
        {
                printf("Failed to open DB\n");
                return -1;
        }
        
	sqlite3_prepare_v2(db, "delete from messages where bbbb = ?1 ", -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, bbbb, -1, SQLITE_STATIC);
	sqlite3_step(stmt); 
        sqlite3_finalize(stmt);

	sqlite3_prepare_v2(db, "insert into messages (bbbb,message,timestamp,age,freq) values ( ?1 , ?2 , ?3 ,'NEW',?4)", -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, bbbb,          -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, message,       -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, timestamp_str, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, freq);
 
	if (sqlite3_step(stmt) == SQLITE_DONE) 
	{ return 0; }
	else
	{ return -2; }
        sqlite3_finalize(stmt);

	sqlite3_close(db);
}



char *get_message_list() {
// create a cJSON object
cJSON *json = cJSON_CreateArray();

	long td;
        sqlite3 *db;
        sqlite3_stmt *stmt;


        purge_old_messages();
        sqlite3_open("navtex.db", &db);
        if (db == NULL)
        {
                printf("Failed to open DB\n");
                return ((char *) 0);
        }

        //sqlite3_prepare_v2(db, "select id,bbbb,timestamp,age,freq from messages order by timestamp desc", -1, &stmt, NULL);
        sqlite3_prepare_v2(db, "select messages.id as id,bbbb,timestamp,age,freq from messages, config AS CO1, config AS CO2 where (freq=518 and CO1.tag='stations518' and instr(CO1.value,substr(bbbb,1,1))>0 and CO2.tag='messages518' and instr(CO2.value,substr(bbbb,2,1)) > 0) OR  (freq=490 and CO1.tag='stations490' and instr(CO1.value,substr(bbbb,1,1))>0 and CO2.tag='messages490' and instr(CO2.value,substr(bbbb,2,1)) > 0) order by timestamp desc", -1, &stmt, NULL);



        while (sqlite3_step(stmt) != SQLITE_DONE) {
                int i;
                int num_cols = sqlite3_column_count(stmt);

		cJSON *element = cJSON_CreateObject();
                for (i = 0; i < num_cols; i++)
                {
                        if(strcmp(sqlite3_column_name(stmt,i),"timestamp") == 0)
                        {
			//	printf("time diff: %ld\n",calculate_time_difference((char*)sqlite3_column_text(stmt, i) ) ) ;
			}
                        switch (sqlite3_column_type(stmt, i))
                        {
                        case (SQLITE3_TEXT):
				if(strcmp(sqlite3_column_name(stmt,i),"age")==0) {
        				td = calculate_time_difference((char*)sqlite3_column_text(stmt, 2) );
					if (td > MESSAGE_WEAHTER_FCST_AGE_LIMIT && sqlite3_column_text(stmt, 1)[1] == 'E' ) {
						cJSON_AddStringToObject(element, sqlite3_column_name(stmt,i), "old" );
					}
					else { 
						cJSON_AddStringToObject(element, sqlite3_column_name(stmt,i), (const char *) sqlite3_column_text(stmt, i));
					}
				}
				else {
					cJSON_AddStringToObject(element, sqlite3_column_name(stmt,i), (const char *) sqlite3_column_text(stmt, i));
				}
				if(strcmp(sqlite3_column_name(stmt,i),"bbbb")==0) {
					switch( sqlite3_column_text(stmt, i)[1]) {
				  	case 'A': 	
						cJSON_AddStringToObject(element, "type","NAV Warning" );
						break;	
				  	case 'B': 	
						cJSON_AddStringToObject(element, "type","MET Warning" );
						break;	
				  	case 'C': 	
						cJSON_AddStringToObject(element, "type","ICE report" );
						break;	
				  	case 'D': 	
						cJSON_AddStringToObject(element, "type","Search and Rescue" );
						break;	
				  	case 'E': 	
						cJSON_AddStringToObject(element, "type","MET Forecast" );
						break;	
				  	case 'F': 	
						cJSON_AddStringToObject(element, "type","Pilot message" );
						break;	
				  	case 'G': 	
						cJSON_AddStringToObject(element, "type","AIS message" );
						break;	
				  	case 'H': 	
						cJSON_AddStringToObject(element, "type","Loran C message" );
						break;	
				  	case 'J': 	
						cJSON_AddStringToObject(element, "type","SatNav message" );
						break;	
				  	case 'L': 	
						cJSON_AddStringToObject(element, "type","NAV Warning" );
						break;	
				  	case 'V': 	
						cJSON_AddStringToObject(element, "type","Ntc to Fisherman" );
						break;	
					default:
						cJSON_AddStringToObject(element, "type","Unknown" );
						break;	

					}
				}

                                //printf("sqlite_CT: %s, ", sqlite3_column_text(stmt, i));
                                break;
                        case (SQLITE_INTEGER):
 				cJSON_AddNumberToObject(element, sqlite3_column_name(stmt,i), sqlite3_column_int(stmt, i));
                 //               printf("%d, ", sqlite3_column_int(stmt, i));
                                break;
                        case (SQLITE_FLOAT):
                                break;
                        default:
                                break;
                        }
                }
		cJSON_AddItemToArray(json,element);
        }

        sqlite3_finalize(stmt);

sqlite3_close(db);


// convert the cJSON object to a JSON string
char *json_str = cJSON_Print(json);

// free the cJSON object
cJSON_Delete(json);

return json_str;
}




void purge_old_messages() {

        sqlite3 *db;
        sqlite3_stmt *stmt;
	int purge_list[100];
	int pl_index;
	long td;
	int i;

        sqlite3_open("navtex.db", &db);
        if (db == NULL)
        {
                printf("Failed to open DB\n");
		return;
        }
        pl_index=0;
        sqlite3_prepare_v2(db, "select id,bbbb,timestamp from messages order by timestamp asc", -1, &stmt, NULL);

        while ((sqlite3_step(stmt) != SQLITE_DONE) && (pl_index<(99))) {
        	td = calculate_time_difference((char*)sqlite3_column_text(stmt, 2) );
		if (td > MESSAGE_PURGE_AGE) {
			purge_list[pl_index]=sqlite3_column_int(stmt, 0);
		}
		pl_index++;
        }
        sqlite3_finalize(stmt);
	i=0;
	while ( i < pl_index) {
        	sqlite3_prepare_v2(db, "delete from messages where id = ?1 ", -1, &stmt, NULL);
		sqlite3_bind_int(stmt, 1,purge_list[i]);
		sqlite3_step(stmt); 
		sqlite3_finalize(stmt);
		i++;
        }
	sqlite3_close(db);


return;
}


char *get_message_text(unsigned int m_id) {
// create a cJSON object
cJSON *json = cJSON_CreateObject();


        char sql_str[80];
        sqlite3 *db;

        sqlite3_stmt *stmt;
        sqlite3_open("navtex.db", &db);

        if (db == NULL)
        {
                printf("Failed to open DB\n");
                return ((char *) 0);
        }
	sprintf(sql_str,"select message from messages where id=%u",m_id);
        sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
		cJSON_AddStringToObject(json, sqlite3_column_name(stmt,0), (const char *) sqlite3_column_text(stmt, 0));
        }

        sqlite3_finalize(stmt);


	sprintf(sql_str,"update messages set age='' where id=%u",m_id);
        sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
        sqlite3_step(stmt);

        sqlite3_finalize(stmt);


sqlite3_close(db);


// convert the cJSON object to a JSON string
char *json_str = cJSON_Print(json);

// free the JSON string and cJSON object
//cJSON_free(json_str);
cJSON_Delete(json);

return json_str;
}


void free_json_str(char *json_str)
{
	cJSON_free(json_str);
}


char *get_config() {
// create a cJSON object
cJSON *json = cJSON_CreateArray();


        sqlite3 *db;
        sqlite3_stmt *stmt;
        sqlite3_open("navtex.db", &db);

        if (db == NULL)
        {
                printf("Failed to open DB\n");
                return ((char *) 0);
        }

        sqlite3_prepare_v2(db, "select * from config", -1, &stmt, NULL);

        while (sqlite3_step(stmt) != SQLITE_DONE) {
                int i;
                int num_cols = sqlite3_column_count(stmt);

		cJSON *element = cJSON_CreateObject();
                for (i = 0; i < num_cols; i++)
                {
                        switch (sqlite3_column_type(stmt, i))
                        {
                        case (SQLITE3_TEXT):
				cJSON_AddStringToObject(element, sqlite3_column_name(stmt,i), (const char *) sqlite3_column_text(stmt, i));
                                break;
                        case (SQLITE_INTEGER):
 				cJSON_AddNumberToObject(element, sqlite3_column_name(stmt,i), sqlite3_column_int(stmt, i));
                                break;
                        case (SQLITE_FLOAT):
                                break;
                        default:
                                break;
                        }
                }
		cJSON_AddItemToArray(json,element);
        }

        sqlite3_finalize(stmt);

sqlite3_close(db);


// convert the cJSON object to a JSON string
char *json_str = cJSON_Print(json);

// free the JSON string and cJSON object
//cJSON_free(json_str);
cJSON_Delete(json);

return json_str;
}


char *update_config(char* tag,char* value) {
char sql_statement[200];
cJSON *json = cJSON_CreateArray();

        sqlite3 *db;
        sqlite3_stmt *stmt;
        sqlite3_open("navtex.db", &db);

        if (db == NULL)
        {
                printf("Failed to open DB\n");
                return ((char *) 0);
        }
        sprintf(sql_statement,"update config set value='%s' where tag='%s' ", value, tag);

        sqlite3_prepare_v2(db, sql_statement, -1, &stmt, NULL);

        sqlite3_step(stmt);

        sqlite3_finalize(stmt);

	sqlite3_close(db);


// convert the cJSON object to a JSON string
char *json_str = cJSON_Print(json);

// free the JSON string and cJSON object
//cJSON_free(json_str);
cJSON_Delete(json);

return json_str;
}


