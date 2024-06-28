import os

path = '/home/fahad/Documents/trustLineCTF/forensic/BasicCalculator/py'


FLAG = open(os.path.join(os.path.dirname(__file__), 'flag.txt'), 'r').read().strip()

N_BITS = 384
TIMEOUT = 3*30
MAX_INTERVALS = 1
MAX_QUERIES = 385


import math

def is_prime(n):
    """ Check if a number n is prime. """
    if n <= 1:
        return False
    if n <= 3:
        return True  # 2 and 3 are prime
    if n % 2 == 0 or n % 3 == 0:
        return False
    i = 5
    while i * i <= n:
        if n % i == 0 or n % (i + 2) == 0:
            return False
        i += 6
    return True

def getPrime(bits):
    """ Generate a prime number with 'bits' number of bits. """
    candidate = (1 << bits) + 1  # Start with a number just above 2^bits to ensure it's at least 'bits' bits long
    while True:
        if is_prime(candidate):
            return candidate
        candidate += 2  # Check only odd numbers for efficiency


def main():
    p, q = getPrime(N_BITS//2), getPrime(N_BITS//2)
    N = p * q
    e = 0x10001
    d = pow(e, -1, (p - 1) * (q - 1))

    secret = bytes_to_long(urandom(N_BITS//9))
    c = pow(secret, e, N)

    print(N)
    print(c)

    intervals = []
    queries_used = 0

    while True:
        print("====================================")
        print("           TAX CALCULATOR           ")
        print("====================================")
        print('1. Calculate your tax\n2. Request your tax record\n3. Get secret flag')
        choice = int(input('> '))

        if choice == 1:
            if len(intervals) >= MAX_INTERVALS:
                print('No more entries allowed!')
                continue

            lower = int(input(f'Sales Tax Rate: '))
            upper = int(input(f'Before Tax Price: '))
            price = lower/upper*100
            price = price+upper
            print("After Tax Price: ", price , "$")
            intervals.insert(0, (lower, upper))

        elif choice == 2:
            queries = input('Input your Tax Registration Number: ')
            queries = [int(c.strip()) for c in queries.split(',')]
            queries_used += len(queries)
            if queries_used > MAX_QUERIES:
                print('No more queries allowed!')
                continue

            results = []
            for c in queries:
                m = pow(c, d, N)
                for i, (lower, upper) in enumerate(intervals):
                    in_interval = lower < m < upper
                    if in_interval:
                        results.append(i)
                        break
                else:
                    results.append(-1)

            print(','.join(map(str, results)), flush=True)

            time.sleep(MAX_INTERVALS * (MAX_QUERIES // N_BITS - 1))
        elif choice == 3:
            secret_guess = int(input('Enter secret: '))
            if secret == secret_guess:
                print(FLAG)


main()