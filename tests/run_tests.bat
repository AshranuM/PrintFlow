@echo off
setlocal EnableDelayedExpansion

pushd "%~dp0.."

set /a PASS_COUNT=0
set /a FAIL_COUNT=0

echo ========================================
echo    PRINTFLOW AUTOMATED TEST RUNNER
echo ========================================
echo.

echo [BUILD 1] Compiling normal build (capacity 50)...
gcc -std=c99 -Wall -Wextra -Wpedantic -o printflow.exe main.c queue.c printer.c
if errorlevel 1 (
    echo [FAIL] Normal build failed to compile.
    set /a FAIL_COUNT+=1
    goto :summary
)
echo [PASS] Normal build compiled cleanly.
set /a PASS_COUNT+=1

echo [BUILD 2] Compiling small-capacity build (capacity 3)...
gcc -std=c99 -Wall -Wextra -Wpedantic -DQUEUE_CAPACITY=3 -DHISTORY_CAPACITY=3 -o printflow_small.exe main.c queue.c printer.c
if errorlevel 1 (
    echo [FAIL] Small-capacity build failed to compile.
    set /a FAIL_COUNT+=1
    goto :summary
)
echo [PASS] Small-capacity build compiled cleanly.
set /a PASS_COUNT+=1

echo.
echo [RUN] Executing test inputs...

printflow.exe < tests\t01.txt > tests\out_t01.tmp
if errorlevel 1 (
    echo [FAIL] t01.txt exited with non-zero status.
    set /a FAIL_COUNT+=1
)

printflow_small.exe < tests\t02.txt > tests\out_t02.tmp
if errorlevel 1 (
    echo [FAIL] t02.txt exited with non-zero status.
    set /a FAIL_COUNT+=1
)

printflow.exe < tests\t03.txt > tests\out_t03.tmp
if errorlevel 1 (
    echo [FAIL] t03.txt exited with non-zero status.
    set /a FAIL_COUNT+=1
)

printflow.exe < tests\t04.txt > tests\out_t04.tmp
if errorlevel 1 (
    echo [FAIL] t04.txt exited with non-zero status.
    set /a FAIL_COUNT+=1
)

printflow.exe < tests\t05.txt > tests\out_t05.tmp
if errorlevel 1 (
    echo [FAIL] t05.txt exited with non-zero status.
    set /a FAIL_COUNT+=1
)

echo.
echo [VERIFY] Checking test assertions...

call :check "tests\out_t01.tmp" "No pending jobs in the queue" "TC04 Process empty queue"
call :check "tests\out_t01.tmp" "Print job #1 added for Aarav Sharma" "TC01 Add one print job"
call :check "tests\out_t01.tmp" "Print job #5 added for Kabir Singh" "TC02 Add several print jobs"
call :check "tests\out_t01.tmp" "Printed Job #1" "TC03 FIFO processing job 1"
call :check "tests\out_t01.tmp" "Printed Job #2" "TC03 FIFO processing job 2"
call :check "tests\out_t01.tmp" "User: Diya Patel" "TC03 FIFO processing job 2 user"
call :check "tests\out_t01.tmp" "Cancelled Job #3 (Rohan Verma" "TC06 Cancel middle pending job"
call :check "tests\out_t01.tmp" "Print job #6 added for Ananya Rao" "Demo Step 5 Job #6 created"
call :check "tests\out_t01.tmp" "Showing last 2 of 2 completed jobs" "Demo History shows 2 of 2"
call :check "tests\out_t01.tmp" "Total Submitted Jobs : 6" "TC24 Demo stats Submitted=6"
call :check "tests\out_t01.tmp" "Total Completed Jobs : 2" "TC24 Demo stats Completed=2"
call :check "tests\out_t01.tmp" "Total Cancelled Jobs : 1" "TC24 Demo stats Cancelled=1"
call :check "tests\out_t01.tmp" "Pending Jobs         : 3" "TC24 Demo stats Pending=3"
call :check "tests\out_t01.tmp" "Total Pages Printed  : 5" "TC24 Demo stats Pages=5"
call :check "tests\out_t01.tmp" "Job #1 has already been completed" "TC09 Cancel completed job"
call :check "tests\out_t01.tmp" "Job #99 does not exist" "TC08 Cancel non-existing job"
call :check "tests\out_t01.tmp" "Cancelled Job #4 (Meera Nair" "TC05 Cancel first pending job"
call :check "tests\out_t01.tmp" "Cancelled Job #6 (Ananya Rao" "TC07 Cancel last pending job"
call :check "tests\out_t01.tmp" "Cancelled Job #5 (Kabir Singh" "TC10 Cancel only pending job"

call :check "tests\out_t02.tmp" "Print queue is full (3/3)" "TC11 Queue-full condition"
call :check "tests\out_t02.tmp" "Print job #4 added for User4" "TC12 Circular wrap-around enqueue #4"
call :check "tests\out_t02.tmp" "Print job #5 added for User5" "TC12 Circular wrap-around enqueue #5"
call :check "tests\out_t02.tmp" "Cancelled Job #4 (User4" "TC13 Cancellation after wrap-around"
call :check "tests\out_t02.tmp" "Print job #6 added for User6" "TC14 Job IDs not reused - #6"
call :check "tests\out_t02.tmp" "Showing last 3 of 4 completed jobs" "TC16/TC17 History overflow and total count"
call :check "tests\out_t02.tmp" "Total Pages Printed  : 110" "TC24 Small-build stats pages=110"

call :check "tests\out_t03.tmp" "Invalid choice. Please enter a single digit" "TC21 Invalid menu input"
call :check "tests\out_t03.tmp" "Invalid input. Must be 1-30 characters" "TC19/TC20 Empty or overlong user name"
call :check "tests\out_t03.tmp" "Invalid input. Must be 1-60 characters" "TC19/TC20 Empty or overlong document name"
call :check "tests\out_t03.tmp" "Input too long (exceeds input buffer limit)" "TC20 Overlong input over 127 chars flushed"
call :check "tests\out_t03.tmp" "Invalid page count. Enter an integer between 1 and 500" "TC18 Invalid page input"
call :check "tests\out_t03.tmp" "Print job #2 added for Second User" "TC15 Failed submissions do not consume IDs"
call :check "tests\out_t03.tmp" "Total Submitted Jobs : 2" "TC25 Failed operations leave state unchanged"

call :check "tests\out_t04.tmp" "End of input reached during job registration" "TC23 EOF during job registration"

call :check "tests\out_t05.tmp" "Print job #1 added for 123456789012345678901234567890" "Boundary 30/60 char and 500 page job"
call :check "tests\out_t05.tmp" "End of input reached. Exiting PrintFlow" "TC22 EOF at menu"

del /q tests\out_t01.tmp tests\out_t02.tmp tests\out_t03.tmp tests\out_t04.tmp tests\out_t05.tmp 2>nul

:summary
echo.
echo ========================================
echo Test Summary: !PASS_COUNT! PASSED, !FAIL_COUNT! FAILED
echo ========================================

popd
if !FAIL_COUNT! gtr 0 exit /b 1
exit /b 0

:check
findstr /c:"%~2" "%~1" >nul
if errorlevel 1 (
    echo [FAIL] %~3 -- expected "%~2"
    set /a FAIL_COUNT+=1
) else (
    echo [PASS] %~3
    set /a PASS_COUNT+=1
)
exit /b 0
