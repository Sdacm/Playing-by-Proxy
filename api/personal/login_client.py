import sys
import json
import getpass
import requests
import os

OLD_HOST = 'https://kessel-api.parsecgaming.com/'

def login(email, password, tfa=''):
	r = requests.post(OLD_HOST + 'v1/auth',
		headers={'Content-Type': 'application/json'},
		data=json.dumps({'email': email, 'password': password, 'tfa': tfa})
	)

	return json.loads(r.text), r.status_code

email = input('Email address: ')
password = getpass.getpass('Password: ')

res, status_code = login(email, password)

if status_code == 403 and res.get('tfa_required'):
	tfa = input('TFA code: ')
	res, status_code = login(email, password, tfa)

print('\n[%d] /v1/auth/' % status_code)

if status_code == 201:
	print('\nsession_id = %s' % res['session_id'])

with open("..\\..\\..\\examples\\client\\launcher.bat", "w") as file:
        file.write("client.exe " + res['session_id'])

#os.chdir("..\\..\\..\\examples\\client")
#os.system("launcher.bat")

print('')
end = input('Press ENTER to close program . . .')
