char *get_message_list();
void get_current_timestamp(char *cts);
char *get_message_text(unsigned int m_id);
char *get_config();
char *update_config(char* tag,char* value); 

int add_message(char *bbbb,char *message, int freq);
char *slots_plan() ;
long calculate_time_difference(char *timestamp);
void free_json_str(char *json_str);
void purge_old_messages();


