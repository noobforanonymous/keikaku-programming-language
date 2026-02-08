import os
import subprocess
import sys
import re

# Colors for output
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

def run_test(compiler_path, test_file):
    print(f"Running {os.path.basename(test_file)}...", end=" ", flush=True)
    
    # Read expected output from file
    expected_output = ""
    with open(test_file, 'r') as f:
        content = f.read()
        match = re.search(r'# Expected:\n((?:#.*\n)*)', content)
        if match:
            expected_output = match.group(1).replace('# ', '').strip()
    
    try:
        process = subprocess.Popen(
            [compiler_path, test_file],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        stdout, stderr = process.communicate(timeout=5)
        
        actual_output = "\n".join([line.strip() for line in stdout.split('\n') if line.strip()])
        expected_output = "\n".join([line.strip() for line in expected_output.split('\n') if line.strip()])
        
        if actual_output == expected_output:
            print(f"{Colors.OKGREEN}PASSED{Colors.ENDC}")
            return True
        else:
            print(f"{Colors.FAIL}FAILED{Colors.ENDC}")
            print(f"\n{Colors.BOLD}Expected:{Colors.ENDC}\n{expected_output}")
            print(f"{Colors.BOLD}Actual:{Colors.ENDC}\n{actual_output}")
            if stderr:
                print(f"{Colors.BOLD}Error:{Colors.ENDC}\n{stderr}")
            return False
            
    except Exception as e:
        print(f"{Colors.FAIL}ERROR{Colors.ENDC}")
        print(f"Detail: {str(e)}")
        return False

def main():
    # Attempt to find the compiler
    compiler_path = "./build/compiler/keikaku"
    if not os.path.exists(compiler_path):
        compiler_path = "./build/keikaku" # Fallback
    
    if not os.path.exists(compiler_path):
        print(f"{Colors.FAIL}Error: Keikaku compiler not found at {compiler_path}{Colors.ENDC}")
        print("Please build the project first.")
        sys.exit(1)

    test_dir = "./tests/suite"
    if not os.path.exists(test_dir):
        print(f"{Colors.FAIL}Error: Test suite directory {test_dir} not found.{Colors.ENDC}")
        sys.exit(1)

    tests = [os.path.join(test_dir, f) for f in os.listdir(test_dir) if f.endswith('.kei')]
    tests.sort()

    print(f"{Colors.HEADER}{Colors.BOLD}Keikaku Test Protocol Alpha{Colors.ENDC}")
    print(f"Compiler: {compiler_path}")
    print(f"Found {len(tests)} tests.\n")

    passed = 0
    for test in tests:
        if run_test(compiler_path, test):
            passed += 1

    print(f"\n{Colors.BOLD}Results: {passed}/{len(tests)} passed.{Colors.ENDC}")
    
    if passed == len(tests):
        print(f"{Colors.OKGREEN}System stable. All according to plan.{Colors.ENDC}")
        sys.exit(0)
    else:
        print(f"{Colors.FAIL}Anomalies detected in core logic.{Colors.ENDC}")
        sys.exit(1)

if __name__ == "__main__":
    main()
