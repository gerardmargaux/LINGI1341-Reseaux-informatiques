#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>

/*
 * Checks if the string passed as argument represents a number
 */
int isnumber(char* string){
    unsigned int i = 0;

    // Check si le nombre n'est pas en hexadécimal
    if(strlen(string) > 2 && *string == '0' && *(string+1) == 'x'){
        return 1;
    }

    while(i < strlen(string)){
        if(isdigit((int) *(string+i)) == 0 && *(string+i) != '.'){
            return 0;
        }
		i++;
    }
    return 1;
}

int main(int argc, char *argv[]){
    extern char* optarg;
    extern int optind, opterr, optopt;
    char* options = "a:s:m:d:ID";
    double res = 0;
    int prec = 0;
	char c;
    while((c = (char) getopt(argc,argv,options)) != -1){
        switch (c){
            case 'a':
                if(isnumber(optarg) == 1){
                    res = res + atof(optarg);
                }
                else{
                    fprintf(stderr, "L'argument n'est pas un nombre.\n");
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                if(isnumber(optarg) == 1){
                    res = res - atof(optarg);
                }
                else{
                    fprintf(stderr, "L'argument n'est pas un nombre.\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'm':
                if(isnumber(optarg) == 1){
                    res = res * atof(optarg);
                }
                else{
                    fprintf(stderr, "L'argument n'est pas un nombre.\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'd':
                if(isnumber(optarg) == 1){
                    if(atof(optarg) == 0){
                        fprintf(stderr, "Seul Chuck Norris peut diviser par 0.\n");
                        return EXIT_FAILURE;
                    }
                    res = res / atof(optarg);
                }
                else{
                    fprintf(stderr, "L'argument n'est pas un nombre.\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'I':
                res++;
                break;
            case 'D':
                res--;
                break;
            default:
                fprintf(stderr, "Option inconnue.\n");
                return EXIT_FAILURE;
        }
    }
    if(optind < argc){
        prec = atoi(argv[optind]);
    }
    fprintf(stdout,"%.*f\n", prec, res);
    return EXIT_SUCCESS;
}
