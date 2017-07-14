#! /bin/bash

mkdir _common_files
pattern='s/\//\\\//g; s/\./\\\./g'
#pattern='s/\ /%20/g; s/\//\\\//g; s/\./\\\./g'

# перед запуском проверь, нужна ли перекодировка, и если не нужна - отключи

echo files:
for mv_file in "$@"; do
	tmp="$(echo "$mv_file" | sed "$pattern")"
	echo "$mv_file  --->  $tmp"
done

echo moves:
#html_files="$"
for html_file in  *.htm; do
	html_dir="${html_file%.htm}_files"
	tmp="$(echo "$html_dir" | sed "$pattern")"
	echo "$html_dir  --->  $tmp"
	
	iconv -f CP1251 -t UTF-8 "$html_file" > tmp
	sed 's/charset=windows-1251/charset=utf-8/' tmp > "$html_file"
	
	for mv_file in "$@"; do
		cp "$html_dir/$mv_file" "_common_files/$mv_file"
		from_pattern="$(   echo "$html_dir/$mv_file" | sed "$pattern")"
		to_pattern="$(echo "$_common_files/$mv_file" | sed "$pattern")"
		#echo "$from_pattern" "$html_file"
		#grep -on "$from_pattern" "$html_file"
		sed -i  "s/$from_pattern/$to_pattern/g" "$html_file"
	done
done
for html_file in  *.html; do
	html_dir="${html_file%.html}_files"
	tmp="$(echo "$html_dir" | sed "$pattern")"
	echo "$html_dir  --->  $tmp"

	iconv -f CP1251 -t UTF-8 "$html_file" > tmp
	sed 's/charset=windows-1251/charset=utf-8/' tmp > "$html_file"

	for mv_file in "$@"; do
		#cp "$html_dir/$mv_file" "_common_files/$mv_file"
		from_pattern="$(   echo "$html_dir/$mv_file" | sed "$pattern")"
		to_pattern="$(echo "$_common_files/$mv_file" | sed "$pattern")"
		#echo "$from_pattern" "$html_file"
		grep -on "$from_pattern" "$html_file"
		#sed -i  "s/$from_pattern/$to_pattern/g" "$html_file"
	done
done

rm tmp