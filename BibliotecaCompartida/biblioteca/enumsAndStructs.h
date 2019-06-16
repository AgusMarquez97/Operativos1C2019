#ifndef ENUMSANDSTRUCTS_H_
#define ENUMSANDSTRUCTS_H_

typedef enum {

	SELECT = 1, INSERT = 2, DESCRIBE = 3, CREATE = 4, DROP = 5, JOURNAL = 6, ADD = 7, RUN = 8

} queryType;


typedef enum {

	SC  =  1, /*	Strong Consistency	*/
	SHC =  2, /*	Strong Hash Consistency	*/
	EC  =  3  /*	Eventual Consistency	*/

} consistencyType;


typedef struct {

	int32_t requestType;
	char* tabla;
	int32_t key;
	char* value;
	int64_t timestamp;
	int32_t consistencyType;
	char * script;
	int32_t cantParticiones;
	int64_t compactationTime;
	
}query;

//Faltarian los parametros de create -> Ver de hacer una estructura por cada request



#endif /* ENUMSANDSTRUCTS_H_ */
