#ifndef ENUMSANDSTRUCTS_H_
#define ENUMSANDSTRUCTS_H_

typedef enum {

	SELECT = 1, INSERT = 2, DESCRIBE = 3, CREATE = 4, DROP = 5, JOURNAL = 6, ADD = 7

} queryType;

typedef struct {

	int32_t requestType;
	char* tabla;
	int32_t key;
	char* value;
	int64_t timestamp;
} query;

//Faltarian los parametros de create -> Ver de hacer una estructura por cada request



#endif /* ENUMSANDSTRUCTS_H_ */
