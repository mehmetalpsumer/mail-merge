# Mail Merge
Mail merge helps you to create copies of a mail with different names,
addresses etc.

## Usage
 Simply run the command on your terminal:
 `make`
 *Make sure you are on correct directory.*
 
 Put the template in an XML file with a root and each placeholder
as an empty XML tag. Create your another XML where each elements
have an ID.

 Program takes 3 arguments, template xml, variables xml and an
output folder address.

`./mm -v variables.xml -t template.xml -o ~/mails`

Each record will be printed with their IDs as txt format.

 ## Reqiurements
 - libxml2
