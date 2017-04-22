#include <stdio.h>
#include <libxml/parser.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "array.h" // array(dynamic sized) header with custom functions

#define BUFFER_SIZE 256 // define char[] size for inputs

// FUNCTION PROTOTYPES
void parseTemplate(char *path);
void parseVariables(char *path);
void recursiveParse(xmlNode *node);
char *formatChar(char *key, char *val);
void writeToFile();
static void _mkdir(const char *dir);

// GLOBAL VARIABLES
/*
 * variables: stores values for each record
 * placeholder_keys: stores only the tag names in template
 * template: stores the text to be written
 */
Array variables, placeholder_keys, template;
char output_path[BUFFER_SIZE]; // output directory

int main(int argc, char **argv) {
    bool template_found = false;
    bool variable_found = false;
    bool output_found = false; // to check if those values are entered
    char template_path[BUFFER_SIZE], variable_path[BUFFER_SIZE];

    // Parsing arguments
    if(argc != 7){
        printf("You did not enter valid number of arguments.\n");
        return 0;
    }
    else{
        int i=1;
        for(i=1; i<argc; i++){
            if(strcmp(argv[i], "-v") == 0){
                if(argc == (i+1) || argv[i+1][0] == '-'){
                    printf("You put the flag '-v' but did not enter a variable XML file.\n");
                    return 0;
                }
                else if(variable_found){
                    printf("You have entered multiple '-v'\n");
                    return 0;
                }
                else{
                    strcpy(variable_path, argv[i+1]);
                    variable_found = true;
                }
            }
            else if(strcmp(argv[i], "-t") == 0){
                if(argc == (i+1) || argv[i+1][0] == '-'){
                    printf("You put the flag '-t' but did not enter a template XML file.\n");
                    return 0;
                }
                else if(template_found){
                    printf("You have entered multiple '-t'\n");
                    return 0;
                }
                else{
                    strcpy(template_path, argv[i+1]);
                    template_found = true;
                }
            }
            else if(strcmp(argv[i], "-o") == 0){
                if(argc == (i+1) || argv[i+1][0] == '-'){
                    printf("You put the flag '-o' but did not enter an output folder path.\n");
                    return 0;
                }
                else if(output_found){
                    printf("You have entered multiple '-o'\n");
                    return 0;
                }
                else{
                    strcpy(output_path, argv[i+1]);
                    output_found = true;
                }
            }
            else{
                // nothing to do for now
                continue;
            }
        }
    }

    // create the output folder
    struct stat st = {0};
    if(output_found && stat(output_path, &st) == -1)
        _mkdir(output_path);

    // start proceeding
    parseTemplate(template_path);
    parseVariables(variable_path);
    printf("Operations are completed.\n");
    return 0;
}

// METHODS
/*
 * parse the given template file and store the whole text
 * and discretely store placeholders
 */
void parseTemplate(char *path){
    arrayDefine(&placeholder_keys);
    arrayDefine(&template);
    xmlDoc *template_xml = xmlReadFile(path, NULL, 256);

    /*
     * this template must have one tag above the template
     * <root> text </root> - VALID
     * <root><class> text </class></root> - INVALID
     */
    if(template_xml!=NULL) {
        xmlNode *template_root = xmlDocGetRootElement(template_xml);
        xmlNode *template_node = template_root->children;
        while (template_node != NULL) {
            if (template_node->type == 1) {
                char *tmp;
                tmp = (char *) malloc(strlen((char *) template_node->name) + 2);
                strcpy(tmp, "^"); // indicator that is a placeholder
                strcat(tmp, (char *) template_node->name);
                // add the node name to placeholder array
                arrayAppend(&placeholder_keys, (char *) template_node->name);
                // add '^nodename' to template array
                arrayAppend(&template, tmp);
            } else {
                // if it is a simple text then add directly to template array
                arrayAppend(&template, (char *) template_node->content);
            }
            template_node = template_node->next;
        }
    }
    else{
        printf("Template xml file could not be read!\n");
        exit(1);
    }
}
/*
 * read the variables xml here and prepare for the recursive func
 */
void parseVariables(char *path){
    xmlDoc *variables_xml = xmlReadFile(path, NULL, 256);
    if(variables_xml!=NULL) {
        xmlNode *variables_root = xmlDocGetRootElement(variables_xml);
        arrayDefine(&variables);
        recursiveParse(variables_root);
    }
    else{
        printf("Variables xml file could not be read!\n");
        exit(1); // abort the program if cant read
    }
}
/*
 * recursively parsing variable xml
 * first attributes, then children, then siblings
 */
void recursiveParse(xmlNode *node){
    bool flag=false; //is this the parent node of a record
    // attributes
    if(node->type==1 && node && node->properties){
        xmlAttr* attribute = node->properties;
        //visit all attributes
        while(attribute && attribute->name && attribute->children)
        {
            // if it is in the template then add it
            if(arrayExists(&placeholder_keys, (char *) attribute->name)
                    && !arrayKeyExists(&variables, (char *) attribute->name)) {
                arrayAppend(&variables, formatChar((char *) attribute->name,
                        (char*)xmlNodeListGetString(node->doc, attribute->children, 1)));
            }
            attribute = attribute->next;
        }
    }
    if(node->children!=NULL){
        if(node->children->type == XML_ELEMENT_NODE){
            recursiveParse(node->children); // if child is node, parse it first
        }
        else if(node->children->type == XML_TEXT_NODE){
            /*
             * if it is not added before and in the given template
             * then add to variables
             */
            if(arrayExists(&placeholder_keys, (char *) node->name)
                    && !arrayKeyExists(&variables, (char *) node->name)){
                arrayAppend(&variables, formatChar((char *) node->name, (char *) node->children->content));
            }
        }

        // checks if it is a parent
        if(node->children->children!=NULL && node->children->children->type==3)
            flag = true;
    }
    /*
     * if it is a parent node (end of a record), then write it to txt file
     * first check if it has all placeholders by checking size
     * since only valid ones are added to array, they should be equal
     */
    if(flag){
        if(variables.size == placeholder_keys.size){
            writeToFile();
        }
        // reset the variable array
        arrayFree(&variables);
        arrayDefine(&variables);
    }
    // sibling
    if(node->next!=NULL){
        recursiveParse(node->next);
    }
}
/*
 * When a record(i.e: student) is finished writes the
 * current values to file, and resets the variable array.
 */
void writeToFile(){
    // declaring file name "id.txt"
    char *id, *ext, *file_path;
    file_path = strdup(output_path);
    // adding '/' to reach directory if not declared in the input
    if(file_path[strlen(file_path)-1]!='/')
        strcat(file_path,"/");
    ext = ".txt";
    id = arrayFind(&variables, "id");
    strcat(id, ext);
    strcat(file_path, id);

    // open a file to write
    FILE *file = fopen(file_path, "w");
    if(file == NULL){
        printf("Error creating .txt!\n");
        exit(1);
    }
    else{
        /*
         * This loop goes for every element of template and writes if an element doesnt start with "^"
         * if an element declared as placeholder then writing the value found on variables array
         * otherwise writing as plain text
         */
        int i;
        for(i=0; i<template.size; i++){
            //check if it is a placeholder
            if(arrayGet(&template, i)[0]=='^'){
                fputs(arrayFind(&variables, strpbrk(arrayGet(&template, i), "^") + 1), file);
            }
            else{
                fputs(arrayGet(&template, i), file);
            }
        }
    }

    fclose(file);
}
/*
 * Taking tags from variables and formats them as
 * value:key to parse easily later.
 */
char *formatChar(char *key, char *val){
    char *tmp;
    tmp = strdup(val);
    strcat(tmp, ":");
    strcat(tmp, key);
    return tmp;
}

/*
 * Creating directory recursively
 * To allow input '-o ../somedir'
 */
static void _mkdir(const char *dir) {
        char tmp[256];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);
}
