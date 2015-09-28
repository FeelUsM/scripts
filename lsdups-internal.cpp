#include <iostream>
#include <fstream>
#include "../projs/common-parse-lib/strin.h"
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <utility>//pair
#include <iterator>//iterator_traits,inserters,movers
#include <limits.h>
using std::cerr;
using std::cout;
using std::endl;
using namespace str;
using std::pair;
using std::make_pair;
using std::string;
using std::vector;
using std::multimap;
using std::map;

//удаляет все, что не повторяется
template<class cont_t, typename comp_t>
void antiuniq(cont_t * pset, const comp_t & comp)
{
	typedef typename cont_t::iterator it_t;
	if(pset->begin()==pset->end())
		return;
	it_t l,r;
	l=r=pset->begin();
	r++;
	bool fl=false;//в первой итерации первый и (не существующий) минус первый элементы НЕ равны
	while(r!=pset->end()){
		if(comp(*l,*r))//если равны
			fl=true;//в будущем - были равны
		else{//если не равны
			if(!fl){//и не были равны
				pset->erase(l);//удаляем того, кто не равен ни левому ни правому
			}
			fl=false;//в будущем они не были равны
		}
		l=r;
		r++;
	}
	if(!fl)//если последний и предпоследний(может не существовать) элементы не равны
		pset->erase(l);
}
template<class cont_t>
void antiuniq(cont_t * pset){
	antiuniq(pset, [](const typename cont_t::value_type & l, const typename cont_t::value_type & r){return l==r;});
}

//удаляет все, что не повторяется
template<class init_t, class outit_t, typename comp_t>
outit_t antiuniq_copy(init_t first, init_t last, outit_t out, const comp_t & comp)
{
	if(first==last) return out;
	init_t l = first;	first++;
	bool f1 = false;
	while(first!=last){
		if(comp(*l,*first)){
			if(!f1){
				*out++ = *l;
				f1=true;
			}
			*out++ = *first;
		}
		else
			f1 = false;
		l++;
		first++;
	}
	return out;
}
template<class init_t, class outit_t>
outit_t antiuniq_copy(init_t first, init_t last, outit_t out)
{
	typedef decltype(*first) val_t;
	return antiuniq_copy(first,last,out,[](const val_t & l, const val_t & r){return l==r;});
}

//работа с экранированными и неэкранированными строками
char * quote_strcpy(char * to, const char * from){
	while(*from){
		if(*from=='"')
			*to++='\\';
		*to++=*from++;
	}
	*to='\0';
	return to;
}
size_t quote_strlen(const char * from){
	size_t rez=strlen(from);
	while(from=strchr(from,'"'))
		rez++, from++;
	return rez;
}

template <class it_t, class mes_t>
void dump(it_t tmp, mes_t mes){
	string s;
	read_fix_length(tmp,50,&s);
	cerr <<mes <<dump(s.c_str()) <<endl;
	//это не рекурсия, это в forward_stream.h определено для дебага, и здесь тоже пригодилось
}

struct quote_out{
	const char * s;
quote_out(const char * ss):s(ss){}
};
std::ostream & operator<<(std::ostream & str, quote_out q){
	const char * s=q.s;
	while(*s){
		if(*s=='\"' || *s=='\\' || *s=='$')
			str<<'\\';
		str <<*s++;
	}
	return str;
}
/*
из <limits.h> получить ARG_MAX, из environ??? получить размер среды, вычесть его, вычесть еще 1000, и создать такого размера строку, и в нее дописывать 
env | wc -c
openssl md5 "файл" "файл" файл...	

отступление:
	рекурсивное подтягивание хешей из подпапок в текущую папку - отдельная утилитка pull-hashes
	анализ повторов, поиск повторяющихся ПАПОК и др. - отдельная утилитка dups-analys
псевдокод:
	lsdups-internal.cpp файлы хеши > повторы
		повторы:
			\n
			размер
			файл
			файл
			файл
	прочитать файлы (размер дата имя)
	antiuniq<размер>
	прочитать хеши (хеш дата имя)
	выкинуть файлы, у которых уже посчитаны хеши с той же датой
	{
		подготовить строку для вызова
		вызвать и перенаправить к нам
		обработать и добавить в файл с хешами и в наш контейнер
	}
	antiuniq<хеш>
	вывести результат
*/

int main(int argc, const char * argv[]){
	if(argc!=3)
	{	cerr << "usage: sldups-internal files-file hashes-file" << endl; return 1;	}

	//=== считываем файлы (размер дата имя) ===
	typedef pair<string,pair<int,int>> sii_t;
	typedef vector<sii_t> sii_cont;
	sii_cont psd;//path size date
	{
		forward_adressed_stream files(true, new file_on_FILE(argv[1],"r"));
		if(!files){	
			cerr <<"не смог открыть файл "<<argv[1] <<endl;	
			return 1;	
		}
		while(!atend(files.iter())){
			sii_t sii;
			if(!(read_dec(files.iter(),&sii.second.first)
				&&E2F(read_dec(files.iter(),&sii.second.second))
				&&E2F(read_fix_char(files.iter(),'\t'))
				&&E2F(read_until_charclass(files.iter(),spn_crlf,&sii.first).reset())
			))
			{	cerr <<"error in files-file "<<argv[1]<<" at " <<get_linecol(files.iter()) <<endl; return 1;	}
			start_read_line(files.iter());
			psd.push_back(move(sii));
		}
	}
	std::sort(psd.begin(),psd.end(),
		[](const pair<string,pair<int,int>> & l, const pair<string,pair<int,int>> & r){
			return l.second.first<r.second.first;//size
		}
	);
	//=== ищем кандидатов по размеру ===
	map<string,pair<int,int>> psd2;
	antiuniq_copy(make_move_iterator(psd.begin()),make_move_iterator(psd.end()),inserter(psd2,psd2.end()),
		[](const pair<string,pair<int,int>> & l, const pair<string,pair<int,int>> & r){
			return l.second.first==r.second.first;//size
		}
	);
	//=== считываем закешированные хеши (хеш дата имя) ===
	typedef pair<string,pair<string,int>> ssi_t;
	multimap<string,pair<string,int>> hps;//hash path size
	{
		forward_adressed_stream hashes(true, new file_on_FILE(argv[2],"r"));
		if(hashes){	
			while(!atend(hashes.iter())){
				string hash;
				int datetime;
				string path;
				if(!(read_until_char(hashes.iter(),'\t',&hash)
					&&E2F(read_dec(hashes.iter(),&datetime))
					&&E2F(read_fix_char(hashes.iter(),'\t'))
					&&E2F(read_until_charclass(hashes.iter(),spn_crlf,&path).reset())
				))
				{	cerr <<"error in hashes-file at " <<get_linecol(hashes.iter()) <<endl; return 1;	}
				start_read_line(hashes.iter());
				
				auto it = psd2.find(path);
				if(it==psd2.end())	continue;
				if(it->second.second/*date*/ != datetime)	continue;
				hps.insert(move(make_pair(move(hash),make_pair(it->first,it->second.first))));//hash path size
				psd2.erase(it);
			}
		}
		//нет файла - ну и хрен с ним - вычислим
	}	//hashes.close()
	//=== вычисляем НЕзакешированные хеши ===
	cerr<<"-------------"<<endl;
	for(auto it=psd2.begin(); it!=psd2.end(); it++)
		cerr<<it->second.first<<"\t"<<it->first<<endl;
	cerr<<"-------------"<<endl;
	int comstrlen=ARG_MAX-1000;
	{
		forward_adressed_stream envlen(true, new file_on_FILE(popen("env | wc -c","r")));
		if(!envlen){	cerr<<"не смог выполнить 'env | wc -c'"<<endl;	return 1;	}
		int x;
		if(!read_dec(envlen.iter(),&x)){	cerr<<"не смог прочитать число в 'env | wc -c'"<<endl;	return 1;	}
		read_spcs(envlen.iter());
		if(!atend(envlen.iter())){	cerr<<"после прочтения 'env | wc -c' что-то осталось"<<endl;	return 1;	}
		comstrlen-=x;
	}
	//cerr<<"comstrlen="<<comstrlen<<endl;
	std::ofstream hashes(argv[2],std::ofstream::app);
	if(!hashes){	cerr<<"не смог открыть "<<argv[2]<<endl;	return 1;	}
	while(!psd2.empty())
	{
		char array[comstrlen];
		char * begin = array, * end = array+comstrlen;
		const char * com = "openssl md5";
		strcpy(begin,com);
		begin += strlen(com);
		auto it = psd2.begin();
		while(it!=psd2.end() && (size_t)(end-begin)>quote_strlen(it->first.c_str())+3){
			*begin++=' ';
			*begin++='"';
			begin=quote_strcpy(begin,it->first.c_str());
			*begin++='"';
			*begin='\0';
			it++;
		}
		//cerr <<"call "<<array<<endl;
		//system(array);
		//break;
		forward_adressed_stream md5(true,new file_on_FILE(popen(array,"r")));
		if(!md5)	{	cerr<<"не смог вызвать openssl"<<endl;	return 1;	}
		while(read_fix_str(md5.iter(),"MD5(")){
			dump(md5.iter(),"будет прочитано: ");
			while(!read_fix_str(md5.iter(),psd2.begin()->first.c_str())){
				cerr<<"файл "<<psd2.begin()->first<<" не посчитался"<<endl;
				if(it==psd2.begin()){	cerr<<"не можем найти файл"<<endl;	return 1;	}
				psd2.erase(psd2.begin());
			}
			if(!read_fix_str(md5.iter(),")= "))
			{	cerr<<"неправильный синатксис после "<<psd2.begin()->first<<endl;	return 1;	}
			string hash;
			read_until_charclass(md5.iter(),spn_crlf,&hash);
			start_read_line(md5.iter());
			hashes<<hash<<'\t'<<psd2.begin()->second.second<<'\t'<<psd2.begin()->first<<endl;
			hps.insert(move(make_pair(move(hash),make_pair(move(psd2.begin()->first),psd2.begin()->second.first))));
			psd2.erase(psd2.begin());
		}
		while(psd2.begin()!=it){
			cerr<<"файл "<<psd2.begin()->first<<" не посчитался"<<endl;
			psd2.erase(psd2.begin());
		}
		if(!atend(md5.iter()))
		{	cerr<<"ожидался конец файла"<<endl;	}
	}
	cout <<"итого до хешей: "<<hps.size()<<endl;
	//for(auto it=hps.begin(); it!=hps.end(); it++)
	//	cout <<it->first<<'\t'<<it->second.second<<'\t'<<it->second.first<<endl;
	antiuniq(&hps,[](const ssi_t & l, const ssi_t & r){	return l.first==r.first;	});
	cout <<"итого: "<<hps.size()<<endl;
	
	auto sss=hps.begin();
	for(auto it=hps.begin(); it!=hps.end(); it++){
		if(it->first!=sss->first){
			sss=it;
			cout <<"#====================" <<it->second.second<<endl;
		}
		cout <<"#rm \""<<quote_out(it->second.first.c_str())<<"\""<<endl;
	}
}
