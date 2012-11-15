#!/bin/bash

set -e
set -u

CLASS_FILE=$1;

echo "${CLASS_FILE}";

BASE_DIR=`dirname "${CLASS_FILE}"`
BASE_FILE=`basename "${CLASS_FILE}" .class`

echo "$BASE_DIR";
echo "$BASE_FILE";

HOBJ_FILE="${BASE_DIR}/${BASE_FILE}.hobj"

echo "$HOBJ_FILE";

echo "converting ${CLASS_FILE} to ${HOBJ_FILE} ..."

cp "${CLASS_FILE}" "${HOBJ_FILE}"

sed -i -e 's/\(^\s*\)class/\1obj/' "${HOBJ_FILE}"
sed -i -e 's/\(^\s*\)\"clsref\"/\1\"ref\"/' "${HOBJ_FILE}"


#find . -name "*.class" -exec sed -