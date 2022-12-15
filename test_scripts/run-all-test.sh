GET_BINARY_LARGE_RESULT=""
HEAD_BINARY_LARGE_RESULT=""
PUT_BINARY_LARGE_RESULT=""

./test_scripts/get_binary_large.sh
GET_BINARY_LARGE_RESULT=$?

./test_scripts/head_binary_large.sh
HEAD_BINARY_LARGE_RESULT=$?

./test_scripts/put_binary_large.sh
PUT_BINARY_LARGE_RESULT=$?


echo ""
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo ""
echo "RESULTS:     0 = Passed   ,   1 = Fail"
echo ""
echo "get_binary_large.sh = ${GET_BINARY_LARGE_RESULT}"
echo "head_binary_large.sh = ${HEAD_BINARY_LARGE_RESULT}"
echo "put_binary_large.sh = ${PUT_BINARY_LARGE_RESULT}"
echo ""
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo ""

