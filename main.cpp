#include "Application.h"

int main(int argc, char *argv[])
{
    if (argc!=2){
        printf("Usage: %s <shared dir>\n", argv[0]);
        return 0;
    }
    Application app(argv[1]);
    app.run();
	return 0;
}
