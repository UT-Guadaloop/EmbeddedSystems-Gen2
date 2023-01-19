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

1. clone repository to your local machine. Make sure to copy repository close to the root directory of your machine to avoid errors from long file names from FreeRTOS kernel. i.e clone close to your 'C:\' directory.

2. Install Code Composer Studio and make sure you add the MSP432 drivers when installing it.

3. You must change some environment variables on the following projects:
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

4. Try to build project, it should build successfully.
5. Connect MSP432 to your laptop via a USB cable and then flash the code from Code Composer Studio.

## How to Contribute

1. Select an issue to work on and create a branch for that issue.

2. write code to solve the issue.

3. Make sure that all projects you worked on still build successfully and test all changes made.

4. Squash any commits you made in the branch into one *descriptive* commit.

5. Make a pull request to the master branch.

6. Make any changes to branch if changes are requested in pull request.

7. Once pull request is approved, merge to master branch


