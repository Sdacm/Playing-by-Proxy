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


API_HOST = 'https://kessel-api.parsecgaming.com/'

sessionID = res['session_id']

def hosts(mode, public):
	r = requests.get(API_HOST + 'v2/hosts?mode=%s&public=%s' % (mode, 'true' if public else 'false'),
		headers={'Content-Type': 'application/json', 'Authorization': 'Bearer %s' % res['session_id']}
	)

	return json.loads(r.text), r.status_code

res, status_code = hosts('desktop', False)

print('\n[%d] /v2/hosts/' % status_code)

if status_code == 200:
	print('\n{0:<23} {1}'.format('NAME', 'PEER_ID'))
	print('{0:<23} {1}'.format('----', '-------'))
	i = 0
	for host in res['data']:
		print('[%d]{0:<20} {1}'.format(host['name'], host['peer_id']) % i)
		i += 1

index = input('Escolhe o número do host a que te queres ligar: ')
        
peerID = res['data'][int(index)]['peer_id']

with open("..\\..\\..\\examples\\client\\launcher.bat", "w") as file:
        file.write("client.exe {} {}".format(sessionID, peerID))

os.chdir("..\\..\\..\\examples\\client")
os.system("launcher.bat")

print('')
end = input('Press ENTER to close program . . .')
