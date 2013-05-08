#include <mysql.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "safenet.h"

#define SAFENET_URL "http://localhost/safe/interface" 
#define SAFENET_KEY "Web01-key" 

char *safe_url;
char *safe_key;


void get_safenet_env(){
    if (getenv("SAFENET_URL")){
	safe_url = getenv("SAFENET_URL");
    }else{
	safe_url = SAFENET_URL;
    }
    if (getenv("SAFENET_KEY")){
	safe_key = getenv("SAFENET_KEY");
    }else{
	safe_key = SAFENET_KEY;
    }
}

static void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if (ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

static size_t
result_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize= size * nmemb;
  struct st_curl_results *res= (struct st_curl_results *)data;

  res->result= (char *)myrealloc(res->result, res->size + realsize + 1);
  if (res->result)
  {
    memcpy(&(res->result[res->size]), ptr, realsize);
    res->size += realsize;
    res->result[res->size]= 0;
  }
  return realsize;
}

/* ------------------------ safenet encrypt ----------------------------- */

my_bool safenet_encrypt_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  st_curl_results *container;

  //if (args->arg_count != 2)
  if (args->arg_count != 1)
  {
    strncpy(message,
            "two arguments must be supplied: safenet_encrypt('<data>').",
            MYSQL_ERRMSG_SIZE);
    return 1;
  }
  get_safenet_env(); 
  args->arg_type[0]= STRING_RESULT;

  initid->max_length= CURL_UDF_MAX_SIZE;
  container= (st_curl_results *)malloc(sizeof(st_curl_results));

  initid->ptr= (char *)container;

  return 0;
}

char *safenet_encrypt(UDF_INIT *initid, UDF_ARGS *args,
                __attribute__ ((unused)) char *result,
               unsigned long *length,
                __attribute__ ((unused)) char *is_null,
                __attribute__ ((unused)) char *error)
{

  CURLcode retref;
  CURL *curl;

  char *fields;
  asprintf(&fields, "mode=encrypt&keyname=%s&input=%s", safe_key, args->args[0]);

  st_curl_results *res= (st_curl_results *)initid->ptr;

  curl_global_init(CURL_GLOBAL_ALL);
  curl= curl_easy_init();

  res->result= NULL;
  res->size= 0;

  if (curl)
  {
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "Expect:");  
  
    //curl_easy_setopt(curl, CURLOPT_URL, args->args[0]);
    curl_easy_setopt(curl, CURLOPT_URL, safe_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, result_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)res);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "safenet/1.0 by netkiller <netkiller@msn.com>");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
    retref= curl_easy_perform(curl);
    if (retref) {
      fprintf(stderr, "error\n");
      strcpy(res->result,"");
      *length= 0;
    }
  }
  else
  {
    strcpy(res->result,"");
    *length= 0;
  }
  curl_easy_cleanup(curl);
  *length= res->size;
  return ((char *) res->result);
}

void safenet_encrypt_deinit(UDF_INIT *initid)
{
  /* if we allocated initid->ptr, free it here */
  st_curl_results *res= (st_curl_results *)initid->ptr;

  free(res->result);
  free(res);
  return;
}

/* ------------------------ safenet decrypt ----------------------------- */

my_bool safenet_decrypt_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  st_curl_results *container;

  //if (args->arg_count != 2)
  if (args->arg_count != 1)
  {
    strncpy(message,
            "two arguments must be supplied: safenet_decrypt('<data>').",
            MYSQL_ERRMSG_SIZE);
    return 1;
  }

  get_safenet_env();

  args->arg_type[0]= STRING_RESULT;

  initid->max_length= CURL_UDF_MAX_SIZE;
  container= (st_curl_results *)malloc(sizeof(st_curl_results));

  initid->ptr= (char *)container;

  return 0;
}

char *safenet_decrypt(UDF_INIT *initid, UDF_ARGS *args,
                __attribute__ ((unused)) char *result,
               unsigned long *length,
                __attribute__ ((unused)) char *is_null,
                __attribute__ ((unused)) char *error)
{

  CURLcode retref;
  CURL *curl;

  char *fields;
  asprintf(&fields, "mode=decrypt&keyname=%s&input=%s", safe_key, args->args[0]);
  st_curl_results *res= (st_curl_results *)initid->ptr;

  curl_global_init(CURL_GLOBAL_ALL);
  curl= curl_easy_init();

  res->result= NULL;
  res->size= 0;

  if (curl)
  {
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "Expect:");  
  
    //curl_easy_setopt(curl, CURLOPT_URL, args->args[0]);
    curl_easy_setopt(curl, CURLOPT_URL, safe_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, result_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)res);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "safenet/1.0 by netkiller <netkiller@msn.com>");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
    retref= curl_easy_perform(curl);
    if (retref) {
      fprintf(stderr, "error\n");
      strcpy(res->result,"");
      *length= 0;
    }
  }
  else
  {
    strcpy(res->result,"");
    *length= 0;
  }
  curl_easy_cleanup(curl);
  *length= res->size;
  return ((char *) res->result);
}

void safenet_decrypt_deinit(UDF_INIT *initid)
{
  /* if we allocated initid->ptr, free it here */
  st_curl_results *res= (st_curl_results *)initid->ptr;

  free(res->result);
  free(res);
  return;
}

/* ------------------------ safenet config ----------------------------- */

my_bool safenet_config_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{

    get_safenet_env();
    return 0;
}

char *safenet_config(UDF_INIT *initid, UDF_ARGS *args,
                __attribute__ ((unused)) char *result,
               unsigned long *length,
                __attribute__ ((unused)) char *is_null,
                __attribute__ ((unused)) char *error)
{

  char *config;
  asprintf(&config, "SAFENET_URL=%s, SAFENET_KEY=%s", safe_url, safe_key);
  return ((char *)config);
}

void safenet_config_deinit(UDF_INIT *initid)
{
   return;
}
