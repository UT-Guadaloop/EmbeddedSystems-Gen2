# Guadaloop's Gen 2 Embedded Systems Repository
This repository contains all of embedded system's existing code for Gen 2.

## Contents
This repository currently contains the following:

- **CCS:** Code Composer Studio workspace containing multiple projects with code for the Gen 2 code. such projects are:
    - <ins>VCU:</ins> contains code to run VCU unit.
    - <ins>HubUnit_front:</ins> contains code to run front hub unit.
    - <ins>HubUnit_Center:</ins> contains code to run center hub unit.
    - <ins>HubUnit_Rear:</ins> contains code to run rear hub unit.
    - <ins>Guadaloop_lib:</ins> contains helper and common methods used throughout all projects. intended to reduce duplicated code.
    - <ins>FreeRTOS:</ins> FreeRTOS project necessary to use FreeRTOS in other projects. 

- **FreeRTOS:** FreeRTOS kernel and source code, no need to install FreeRTOS separately. 

## How to start
1. Install the [MSP432 SDK](https://www.ti.com/tool/download/SIMPLELINK-MSP432E4-SDK) in your machine.
2. Install Code Composer Studio (CCS).
    * Follow section 4 (Quick Start for CCS IDE ) of this [guide](https://github.com/UT-Guadaloop/EmbeddedSystems-Gen2/blob/groundstation-changes/Resources/MSP432SDK_Guide.pdf) for exact steps. 

3. clone repository to your local machine. Make sure to copy repository close to the root directory of your machine to avoid errors from long file names from FreeRTOS kernel. i.e clone close to your 'C:\' directory.


4. Open Code Composer Studio workspace on CCS folder.
    * Open Code composer studio. you should get a screen like this: ![CCS_1](/Resources/CCS_1.png)
    
    * Click browse, go to the location of where you cloned the repository and select the "CCS" folder.

    * Finally, click launch.

5. Import the projects from the repository to your workspace.
    * Go to "project->Import CCS Projects": ![CCS_import_projects](/Resources/CCS_import_projects.png)

    * Select the "Select search-directory" option, then click on Browse: ![CCS_search_dir](/Resources/CCS_search_dir.png). Then go to the location where you clones this repository and select the "CCS" folder.

    * You should be able to see the 6 projects as shown below. select all of them, check mark both the "Automatically import referenced projects found in same search-directory" and "Copy projects into workspace" options. then click finish.

    * You should now have all the projects in your workspace.


6. Change Environment variables of projects to be able to build them.
    * You will have to do this for these projects:
        - FreeRTOS
        - HubUnit_Front
        - HubUnit_Center
        - HubUnit_Rear
        - VCU

    #### <ins>Instructions:</ins>
 
    1. right-click on project and click on *properties*.
    2. Go into *Resource->Linked Resources*. 
    3. You should have a ti folder in your computer, once you find it, the path for each TI variable should be the same as the one shown in the image below. Change the FREERTOS_INSTALL_DIR to point to the *FreeRTOS* folder in this repository. Change the COM_TI_SIMPLELINK_MSP432E4_SDK_INSTALL_DIR variable to point to the *MSP432_SDK* folder in the repository.

        ![linked resources page](/Resources/linked_resources.png "linked resources page")
        *image 1: Linked Resources Page*


7. Try to build project, it should build successfully.
8. Connect MSP432 to your laptop via a USB cable and then flash the code from Code Composer Studio.

## How to Contribute

1. Select an issue to work on and create a branch for that issue.

2. write code to solve the issue.

3. Make sure that all projects you worked on still build successfully and test all changes made.

4. Squash any commits you made in the branch into one *descriptive* commit.

5. Make a pull request to the master branch.

6. Make any changes to branch if changes are requested in pull request.

7. Once pull request is approved, merge to master branch


