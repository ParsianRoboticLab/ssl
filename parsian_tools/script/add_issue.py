#!/usr/bin/python3.5

import os
import json
import requests
import sys

# Authentication for user filing issue (must have read/write access to
# repository to add issue to)
if len(sys.argv) < 4:
	print("3 argument is needed")
	exit(1)

USERNAME = str(sys.argv[1])
PASSWORD = str(sys.argv[2])

# The repository to add this issue to
REPO_OWNER = 'parsianroboticlab'
REPO_NAME = 'ssl'

def make_github_issue(title, body=None, labels=[]):
    '''Create an issue on github.com using the given parameters.'''
    # Our url to create issues via POST
    url = 'https://api.github.com/repos/%s/%s/issues' % (REPO_OWNER, REPO_NAME)
    # Create an authenticated session to create the issue
    session = requests.Session()
    session.auth = (USERNAME, PASSWORD)
    # Create our issue
    issue = {'title': title,
             'body': body,
             'labels': labels}
    # Add the issue to our repository
    r = session.post(url, json.dumps(issue))
    if r.status_code == 201:
        print ('Successfully created Issue {0:s}'.format(title))
    else:
        print ('Could not create Issue {0:s}'.format(title))
        print ('Response:', r.content)

bd = 'added by script' if len(sys.argv) == 4 else sys.argv[4]
make_github_issue('ABS_' + str(sys.argv[3]), bd, [])
