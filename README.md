############################# slab-blstore ################################

Please extract the tar ball and you will find 3 folders, slab, blstore and
slab-blstore. The details on how to run each module is described below.
Please let me know if you faced any difficulty while running the programs


slab folder:
------------

This folder contains the implementation of slab-allocator.

Please use the Makefile and slab-tester.c file present in the folder. I have
modified the Makefile.

I have modified the name of 'size' variable, present in the struct slab_query,
to 'obj_size'. I have done this because throughout my code, I have used size
variable name for some other purpose. So instead of changing the variable name
in my code, I changed it in slab_query structure as that was easier. THIS 
RESULTED IN MODIFICATION OF SLAB_TESTER PROGRAM.


To run the slab-allocator -
 - go inside the slab folder
 - run Make command
 - run the program "./slab-tester"

The module is tested sample output is present in the 
program_output file.

blstore folder:
---------------

This folder contains the implementation of working transactional Block Store.
Please use the Makefile file present in the folder. I have modified it. You can
use the bl-test.c file present in this folder or your own copy.

To run the blstore -
 - go inside the blstore folder
 - run Make command
 - run the program "./bl-test -rs 7000 TEST-REPOS"

The module is tested and sample output is present in the 
program_output file.

slab-blstore folder:
--------------------

This folder contains the implementation of slab-blstore. It exlores the idea 
if slab allocation cab be used for Trasactional Block Store to increase the performance
and reduce the internal fragmentation.

Please use the Makefile present in this folder.

THIS PROJECT IS COMPLETELY WORKING EXCEPT CONSTRUCTION AND DESTRUCTION OF OBJECTS

To run the slab-blstore -
 - go inside the slab-blstore folder
 - run Make command
 - run the program "./dslab-tester -rs 7000 TEST-REPOS"

The module is tested on sample output is present in the 
program_output file.



FUTURE WORK:
------------

The storing of the constructor and destructor functions on the blockstore. As 
we had discussion, currently there is no efficient method known to us.


