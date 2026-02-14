import argparse
import subprocess
import time
import random
import shlex
import requests
import signal

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1


def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def run(command, output=None):
    process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)    
    return process


def start_pref_record(pid, output="perf.data"):
   
    #command = f"sudo perf record -o {output} -p {process.pid}"
    command = f'perf record -o {output} -g -p {pid}'
    process = run(command, output=subprocess.DEVNULL)
    time.sleep(1)
    return process


def stop_pref_record(perf_proc, wait=False):
    perf_proc.send_signal(signal.SIGINT)
    perf_proc.wait()    
    #if process.poll() is None and wait:
    #    process.wait()
    #process.terminate()
    
    
def create_flamegraph(inp ="perf.data", output=None):
    #process = subprocess.Popen(shlex.split(f"sudo perf script -i {inp}"))
    command = f'sudo perf script -i {inp} | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > graph.svg'
    process = subprocess.Popen(command, shell=True)
    process.wait()
    return process

def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    process.terminate()


def shoot(ammo):
    hit = run('curl ' + ammo, output=subprocess.DEVNULL) 
    time.sleep(COOLDOWN)
    stop(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')


server = run(start_server())
pref_record = start_pref_record(server.pid)
make_shots()
stop_pref_record(pref_record)
stop(server)
time.sleep(1)
create_flamegraph()
print('Job done')
