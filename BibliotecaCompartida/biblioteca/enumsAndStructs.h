#ifndef ENUMSANDSTRUCTS_H_
#define ENUMSANDSTRUCTS_H_

typedef enum {

	SELECT = 1, INSERT = 2, DESCRIBE = 3, CREATE = 4, DROP = 5, JOURNAL = 6, ADD = 7

} queryType;

typedef struct {

	queryType tipo;
	char* tabla;
	int key;
	char* value;
	int timestamp;
} query;

#endif /* ENUMSANDSTRUCTS_H_ */
