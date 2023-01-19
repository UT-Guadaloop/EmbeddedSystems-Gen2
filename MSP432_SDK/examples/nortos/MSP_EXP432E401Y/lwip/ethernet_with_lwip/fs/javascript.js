<!-- Copyright (c) 2013-2017 Texas Instruments Incorporated.  All rights reserved. -->

window.onload = function()
{
    document.getElementById('overview').onclick = loadOverview;
    document.getElementById('msp432e4').onclick = loadMSP;
    document.getElementById('block').onclick = loadBlock;
    document.getElementById('resources').onclick = loadResources;
    
    loadPage("overview.htm");
}

function loadResources()
{
    loadPage("resources.htm");
    return false;
}

function loadOverview()
{
    loadPage("overview.htm");
    return false;
}

function loadMSP()
{
    loadPage("msp432e4.htm");
    return false;
}

function loadBlock()
{
    loadPage("block.htm");
    return false;
}

function loadPage(page)
{
    if(window.XMLHttpRequest)
    {
        xmlhttp = new XMLHttpRequest();
    }
    else
    {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }

    xmlhttp.open("GET", page, true);
    xmlhttp.setRequestHeader("Content-type",
                             "application/x-www-form-urlencoded");
    xmlhttp.send();

    xmlhttp.onreadystatechange = function ()
    {
        if((xmlhttp.readyState == 4) && (xmlhttp.status == 200))
        {
            document.getElementById("content").innerHTML = xmlhttp.responseText;
        }
    }
}
