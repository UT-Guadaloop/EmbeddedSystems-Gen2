<!-- Copyright (c) 2013-2017 Texas Instruments Incorporated.  All rights reserved. -->

window.onload = function()
{
    document.getElementById('overview').onclick = loadOverview;
    document.getElementById('msp432e4').onclick = loadMSP;
    document.getElementById('block').onclick = loadBlock;
    document.getElementById('io_http').onclick = loadIOHttp;
    document.getElementById('resources').onclick = loadResources;

    loadPage("overview.htm");
}
