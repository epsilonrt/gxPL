#!/bin/sh
TARGET=main_page.dox

PARSER=cat
command -v markdown >/dev/null 2>&1 && PARSER=markdown
echo "PARSER=$PARSER"
if [ "$PARSER" = "markdown" ]; then
  HEAD_TAG="@htmlonly"
  FOOT_TAG="@endhtmlonly"
fi

cat main_page_header.dox > $TARGET

for f in doc*.md
do
  echo "@page $(echo $f | cut -d '-' -f 1) $(head -1 $f)" >> $TARGET
  echo "$HEAD_TAG" >> $TARGET
  tail -n +2 $f | ${PARSER}  >> $TARGET
  echo "$FOOT_TAG" >> $TARGET
done
cat main_page_footer.dox >> $TARGET
