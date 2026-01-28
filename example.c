#define DYN_IMPLEMENTATION
#include "lib.c"

#include<stdio.h>
#include<stdlib.h>

void foo(Dyn_Object* description) {
	if (dyn_eq(dyn_get(description, "func"), dyn_string("print"))) {
		for (int i = 0; i < dyn_to_number(dyn_get(description, "times")); i++) {
			printf("%s", dyn_to_string(dyn_get(description, "string")));
		}
	} else if (dyn_eq(dyn_get(description, "func"), dyn_string("hello"))){
		printf("Hello, %s\n", dyn_to_string(dyn_get(description, "name")));
	}
}

int main() {
	dyn_init(1<<20, NULL);
	Dyn_Object description = dyn_object(
			"func", dyn_string("print"),
			"times", dyn_number(10),
			"string", dyn_string("You are an idiot\n"));
	foo(&description);
	Dyn_Object desc2 = dyn_object("func", dyn_string("hello"), "name", dyn_string("Oleg"));
	foo(&desc2);
	dyn_deinit();
	return 0;
}
