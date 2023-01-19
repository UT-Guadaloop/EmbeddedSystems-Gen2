import os
import sys
import argparse
import requests
from requests.packages.urllib3.exceptions import InsecureRequestWarning

parser = argparse.ArgumentParser()
parser.add_argument("ipaddr", help="The IP address of the device")
parser.add_argument("-s", "--secure", help="Enable secure (https) requests",
                    action="store_true")
parser.add_argument("-u", "--url", help="Specify a particular url to make requests to")

args = parser.parse_args()

# IP passed must match IP assigned to target
os.environ["no_proxy"] = args.ipaddr

if args.secure:
    base = 'https://' + args.ipaddr + ':443'
    print("Making secure requests")
else:
    base = 'http://' + args.ipaddr + ':80'

# Print out a response
def dump(r):
    print(str(r.status_code) + " - " + str(r.headers))
    try:
        print("(raw text)")
        print(r.text)
    except:
        print("(exception - raw text)")
        print(r.text)
    print("\n")

# Print out a response if it is not as expected
def dumpOnErr(r, status_code, response = "", headers = None):
    if (headers == None):
        # provide a good default based on response string
        headers = {'Content-Type': 'text/plain',
                   'Content-Length': str(len(response))}

    if (r.status_code != status_code):
        print("Error, expected " + str(status_code))
        dump(r)
    elif (r.text != response):
        print("Error, expected " + response)
        dump(r)
    elif (r.headers != headers):
        print("Error in headers, expected " + str(headers))
        dump(r)

if args.url is None:
    try:
        # Begin requests
        s = requests.Session()
        # Due to the nature of the dummy certificate provided, we will not verify it
        s.verify = False
        requests.packages.urllib3.disable_warnings(InsecureRequestWarning)
        s.Connection = 'close'

        # Execute a get request for the '/home.html' page
        r = s.get(base + '/home.html')
        resp = "/get 'home.html': This is the resource requested."
        dumpOnErr(r, 200, resp)

        # Execute a get request for the '/login.html' page
        r = s.get(base + '/login.html')
        resp = "/get 'login.html': This is the login page."
        dumpOnErr(r, 200, resp);

        # Post some values to the server
        nameStr = "Tex"
        activityStr = "sleeping"

        payload = {"name":nameStr}
        r = s.post(base + '/db.html', data=payload)
        resp = "Data string successfully posted to server."
        dumpOnErr(r, 200, resp)
        dump(r)

        payload = {"function":activityStr}
        r = s.post(base + '/db.html', data=payload)
        resp = "Data string successfully posted to server."
        dumpOnErr(r, 200, resp)
        dump(r)

        # Retrieve the values just stored in the server
        payload = {"name":"val"}
        r = s.get(base + '/db.html', params=payload)
        resp = nameStr
        dumpOnErr(r, 200, resp)
        dump(r)

        payload = {"function":"val"}
        r = s.get(base + '/db.html', params=payload)
        resp = activityStr
        dumpOnErr(r, 200, resp)
        dump(r)

        # Execute a request for the put-specific URLHandler
        r = s.put(base + '/index.html')
        resp = "URLHandler #2 : /put This is the body of the resource you requested."
        dumpOnErr(r, 200, resp)
        dump(r)

        # Attempt a request the server is not currently configured to handle
        r = s.patch(base + '/db.html')
        resp = "This method is not supported at this URL."
        dumpOnErr(r, 405, resp)
        dump(r)

        print("Testing complete")
        exit(0)
    except Exception as e:
        print("Caught exception - ")
        print(e)
        exit(1)
else:
    s = requests.Session()
    #s.verify = False
    requests.packages.urllib3.disable_warnings(InsecureRequestWarning)
    s.Connection = 'close'

    r = s.get(base + args.url)
    dump(r)
