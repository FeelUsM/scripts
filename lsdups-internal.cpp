#include <iostream>
#include <fstream>
#define ONE_SOURCE
#include <strstr/strin.h>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <utility>//pair
#include <iterator>//iterator_traits,inserters,movers
#include <limits.h>
#include <iomanip>
#include <set>
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
using std::set;
using std::move;

namespace quote_util{
	//работа с экранированными и неэкранированными строками
	char * quote_strcpy(char * to, const char * from){
		while(*from){
			if(*from=='"' || *from=='\\' || *from=='$' || *from=='`' )
				*to++='\\';
			*to++=*from++;
		}
		*to='\0';
		return to;
	}
	size_t quote_strlen(const char * from){
		size_t rez=strlen(from);
		while(from+=strcspn(from,"\"\\$`"), *from)
			rez++, from++;
		return rez;
	}

	struct quote_out{
		const char * s;
	quote_out(const char * ss):s(ss){}
	quote_out(const string & ss):s(ss.c_str()){}
	};
	std::ostream & operator<<(std::ostream & str, quote_out q){
		const char * s=q.s;
		while(*s){
			if(*s=='\"' || *s=='\\' || *s=='$' || *s=='`')
				str<<'\\';
			str <<*s++;
		}
		return str;
	}	
}

namespace io_util{
	template <class it_t, class mes_t>
	void dump(it_t tmp, mes_t mes){
		string s;
		read_line(tmp,&s);
		cerr <<mes <<::str::dump(s.c_str()) <<endl;
		//это не рекурсия, это в forward_stream.h определено для дебага, и здесь тоже пригодилось
	}

	template<class int_t>
	struct bytes_cont{
		int_t size;
		bytes_cont (int_t x):size(x){}
	};
	template<class int_t>
	bytes_cont<int_t> bytes(int_t x){	return bytes_cont<int_t>(x);	}
	template<class int_t>
	std::ostream & operator<<(std::ostream & str, bytes_cont<int_t> b){
		double size2=b.size;
		if(size2/1024<1.1)
			return str<<b.size<<"B";
		size2/=1024;
		if(size2/1024<1.1)
			return str<<std::setprecision(4)<<size2<<"kB";
		size2/=1024;
		if(size2/1024<1.1)
			return str<<std::setprecision(4)<<size2<<"MB";
		size2/=1024;
		if(size2/1024<1.1)
			return str<<std::setprecision(4)<<size2<<"GB";
		return str<<std::setprecision(4)<<size2<<"GB";
	}

	struct datetime{
		time_t tt;
		datetime(time_t tttt):tt(tttt){}
	};
	std::ostream & operator<<(std::ostream & str, datetime dt){
		tm * ptime = localtime(&dt.tt);
		using namespace std;
		return str<<setfill('0')<<setw(2)<<(ptime->tm_year+1900)<<"-"
			<<setfill('0')<<setw(2)<<(ptime->tm_mon+1)<<"-"
			<<setfill('0')<<setw(2)<<(ptime->tm_mday)<<"@"
			<<setfill('0')<<setw(2)<<ptime->tm_hour<<"-"
			<<setfill('0')<<setw(2)<<ptime->tm_min<<"-"
			<<setfill('0')<<setw(2)<<ptime->tm_sec;
	}

	struct total_bytes{
		long long b;
		total_bytes(long long x):b(x){}
	};
	std::ostream & operator<<(std::ostream & str, total_bytes bb){
		int x;
		if(x=bb.b/1024/1024/1024)
			str<<x<<"GB ";
		bb.b%=(1024*1024*1024);
		if(x=bb.b/1024/1024)
			str<<x<<"MB ";
		bb.b%=(1024*1024);
		if(x=bb.b/1024)
			str<<x<<"kB ";
		bb.b%=(1024);
		if(x=bb.b)
			str<<x<<"B";
		return str;
	}
	
	using quote_util::quote_out;
	
	//vector<string>
	std::ostream & operator<<(std::ostream & str, const vector<string> & names){
		auto it=names.cbegin();
		if(it!=names.cend())
			str << quote_out(*it++);
		for(;it!=names.cend(); it++)
			str << '/'<<quote_out(*it);
		return str;
	}
}

typedef pair<string,pair<long long,int>> sii_t;
typedef vector<sii_t> sii_cont;
typedef multimap<string,pair<string,long long>> hps_t;

namespace input{
	int read_files(const char * name_of_files, sii_cont * ppsd){
		forward_adressed_stream files(true, new file_on_FILE(name_of_files,"r"));
		if(!files){	
			cerr <<"не смог открыть файл "<<name_of_files <<endl;	
			return 1;	
		}
		while(!atend(files.iter())){
			sii_t sii;
			if(!(read_dec(files.iter(),&sii.second.first)
				&&E2F(read_dec(files.iter(),&sii.second.second))
				&&E2F(read_fix_char(files.iter(),'\t'))
				&&E2F(read_until_charclass(files.iter(),spn_crlf,&sii.first).reset())
			))
			{	cerr <<"error in file of files "<<name_of_files<<" at " <<get_linecol(files.iter()) <<endl; return 1;	}
			read_start_line(files.iter());
			ppsd->push_back(move(sii));
		}
		return 0;
	}

	int read_hashes(const char * name_of_hashes, 
		map<decltype(sii_t().first),decltype(sii_t().second)> * ppsd2,
		hps_t * phps){
		forward_adressed_stream hashes(true, new file_on_FILE(name_of_hashes,"r"));
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
				read_start_line(hashes.iter());
				
				auto it = ppsd2->find(path);
				if(it==ppsd2->end())	continue;
				if(it->second.second/*date*/ != datetime)	continue;
				phps->insert(move(make_pair(move(hash),make_pair(it->first,it->second.first))));//hash path size
				ppsd2->erase(it);
			}
		}
		else
			cerr <<"файл хешей "<<name_of_hashes<<" отсутствует"<<endl;
		return 0;	//нет файла - ну и хрен с ним - вычислим
	}	//hashes.close()

	int calc_and_write_hashes(const char * name_of_hashes, 
		map<decltype(sii_t().first),decltype(sii_t().second)> * ppsd2,
		hps_t * phps){
		
		using namespace io_util;
			
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
		std::ofstream hashes(name_of_hashes,std::ofstream::app);
		if(!hashes){	cerr<<"не смог открыть "<<name_of_hashes<<endl;	return 1;	}

		long long eval_size=0;
		for(auto it = ppsd2->begin(); it!=ppsd2->end(); it++)
			eval_size+=it->second.first;
		while(!ppsd2->empty())
		{
			using namespace quote_util;
			//cerr<<"===================================================="<<endl
			//	<<"хотим посчитать хеши для следующих файлов"<<endl;
			char array[comstrlen];
			char * begin = array, * end = array+comstrlen;
			const char * com = "openssl md5";
			strcpy(begin,com);
			begin += strlen(com);
			auto it = ppsd2->begin();
			while(it!=ppsd2->end() && (size_t)(end-begin)>quote_strlen(it->first.c_str())+4){
				*begin++=' ';
				*begin++='"';
				begin=quote_strcpy(begin,it->first.c_str());
				eval_size-=it->second.first;
				//cerr<<it->first<<endl;
				*begin++='"';
				*begin='\0';
				it++;
			}
			//cerr<<"------------------------"<<endl<<array<<endl<<"-----------------------------------"<<endl;
			if(eval_size) cerr<<"осталось посчитать "<<bytes(eval_size)<<endl;
			int iter_open=0;
		try_open:
			forward_adressed_stream md5(true,new file_on_FILE(popen(array,"r")));
			if(!md5){
				cerr<<"не смог вызвать openssl"<<endl;
				if(++iter_open<10){
					system("sleep 1");
					goto try_open;
				}
				return 1;
			}
			while(read_fix_str(md5.iter(),"MD5(")){
				//dump(md5.iter(),"будет прочитано: ");
				string hash;
				//cerr <<"перед циклом"<<endl;
				while(
					ppsd2->begin()!=it &&
					!reifnot_E(md5.iter(),read_it,read(read_it)>>fix_str(ppsd2->begin()->first.c_str())
						>>")= ">>fix_length(32,&hash)>>'\n')
				){
					cerr<<"файл "<<ppsd2->begin()->first<<" не посчитался"<<endl;
					ppsd2->erase(ppsd2->begin());
				}
				//cerr <<"после цикла"<<endl;
				if(ppsd2->begin()==it){	dump(md5.iter(),"не понятно, что посчитал openssl md5:\n");	return 1;	}
				hashes<<hash<<'\t'<<ppsd2->begin()->second.second<<'\t'<<ppsd2->begin()->first<<endl;
				phps->insert(move(make_pair(move(hash),make_pair(move(ppsd2->begin()->first),ppsd2->begin()->second.first))));
				ppsd2->erase(ppsd2->begin());
			}
			while(ppsd2->begin()!=it){
				cerr<<"файл "<<ppsd2->begin()->first<<" не посчитался"<<endl;
				ppsd2->erase(ppsd2->begin());
			}
			if(!atend(md5.iter()))
			{	cerr<<"ожидался конец файла"<<endl;	}
		}
		return 0;
	}
}

namespace my_algorithms{
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

	//копирует все, что повторяется
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
	
void print_simple(const hps_t & hps);
void print_script_rmfiles(hps_t * phps);
void print_script_rmdirs(const hps_t * hps, const sii_cont & psd);

// === MAIN ===
int main(int argc, const char * argv[]){
	using namespace io_util;
	using namespace my_algorithms;
	using namespace input;

	int errcode;
	if(argc!=3)
	{	
		cerr << "usage: sldups-internal files-file hashes-file" << endl; 
		cerr<<"geted args:"<<endl;
		for(int i = 1; i<argc; i++)
			cerr <<argv[i]<<endl;
		return 1;	
	}
	const char * name_of_files=argv[1];
	const char * name_of_hashes=argv[2];
	
	//=== считываем файлы (размер дата имя) ===
	sii_cont psd;//path size date - список всех файлов
	
	if(errcode=read_files(name_of_files,&psd))
		return errcode;
	if(psd.empty())	cerr<<"files empty"<<endl;
	std::sort(psd.begin(),psd.end(),
		[](const pair<string,pair<long long,int>> & l, const pair<string,pair<long long,int>> & r){
			return l.second.first<r.second.first;//size
		}
	);
	long long int total_size=0;
	for(auto it=psd.begin(); it!=psd.end(); it++)
		total_size+=it->second.first;
	cerr << "из "<<psd.size()<<" файлов, общим размером "<<bytes(total_size)<<endl;
	
	//=== ищем кандидатов по размеру ===
	map<decltype(sii_t().first),decltype(sii_t().second)> psd2;
	antiuniq_copy(psd.begin(),psd.end(),inserter(psd2,psd2.end()),
		[](const pair<string,pair<long long,int>> & l, const pair<string,pair<long long,int>> & r){
			return l.second.first==r.second.first;//size
		}
	);
	//с этого момента для определения дубликатов список всех файлов больше не нужен
	cerr << psd2.size()<<" файлов с одинаковым размером"<<endl;
	
	//=== считываем закешированные хеши (хеш дата имя) ===
	hps_t hps;//hash path size
	if(errcode=read_hashes(name_of_hashes,&psd2,&hps))
		return errcode;
	//=== вычисляем НЕзакешированные хеши ===
	if(!psd2.empty()){
		cerr <<"у "<<psd2.size()<<" файлов будут посчитаны хеши"<<endl;
		if(errcode=calc_and_write_hashes(name_of_hashes,&psd2,&hps))
			return errcode;
	}
	//с этого момента psd2 пуст

	//=== ищем кандидатов по хешу ===
	antiuniq(&hps,[](const pair<string,pair<string,long long>> & l, const pair<string,pair<string,long long>> & r)
		{	return l.first==r.first;	});
	cerr <<hps.size()<<" файлов c одинаковым MD5"<<endl;

	//=== вызываем анализ и вывод ===
	//print_simple(hps);
	//print_script_rmfiles(&hps);
	std::sort(psd.begin(),psd.end(),
		[](const pair<string,pair<long long,int>> & l, const pair<string,pair<long long,int>> & r){
			return l.first<r.first;//path
		}
	);
	print_script_rmdirs(&hps,psd);
}

void print_simple(const hps_t & hps){
	using namespace io_util;
	//=== вывести простым способом ===
	auto sss=hps.cbegin();
	long long size=0;
	cout<<"///"<<hps.cbegin()->second.second<<endl;
	for(auto it=hps.cbegin(); it!=hps.cend(); it++){
		if(it->first!=sss->first){
			sss=it;
			size-=it->second.second;
			cout<<"///"<<it->second.second<<endl;
		}
		size+=it->second.second;
		cout <<it->second.first.c_str()<<endl;
	}
	cerr<<"итого "<<bytes(size)<<" избыточного"<<endl;
}

pair<const char *, const char *> str_mismatch(const char * s1, const char *s2){
	while(*s1 && *s2 && *s1==*s2)
		s1++, s2++;
	return make_pair(s1,s2);
}

//возвращает расширение
const char * find_ext(const string * ps){
	int dirpos=ps->rfind('/'), extpos=ps->rfind('.');
	if(extpos>=0 && dirpos<extpos)
		return ps->c_str()+extpos;
	else
		return "";
}

void print_script_rmfiles(hps_t * phps){
	using namespace io_util;
	using namespace quote_util;
	map<string,map<string,multimap<long long,set<string>>>> dups;//comm_path,ext,size,diff_path
	long long tot_size=0;
	//=== сгруппировать по стартовым директориям и расширениям ===
																		//cerr<<"///"<<phps->begin()->second.second<<endl;
																		//cerr<<"итого "<<bytes(tot_size)<<" избыточного"<<endl;
	if(phps->begin()!=phps->end()){
		auto sss=phps->begin();
		long long size= (phps->begin()==phps->end()) ? 0 : phps->begin()->second.second;
		set<string> ldups;//файлы
		for(auto it=phps->begin(); true; it++){
			if(it==phps->end() || it->first!=sss->first){//вышли за предел повт. файлов или за предел массива
				tot_size-=sss->second.second;
				sss=it;//первый повт. файл
																			//cerr<<"///"<<it->second.second<<endl;
																			//cerr<<"итого "<<bytes(tot_size)<<" избыточного"<<endl;
				string compath;//общий путь и удаление его из файлов
				{
					auto ff=ldups.begin(), ll=ldups.end();	ll--;
					int comlen=str_mismatch(ff->c_str(),ll->c_str()).first-ff->c_str();
					//cerr <<"from "<<*ff<<endl<<"to "<<*ll<<endl<<"lenis"<<comlen<<endl;
					while(comlen>0 && ff->c_str()[comlen-1]!='/')
						comlen--;
					if(comlen){
						compath.append(ff->c_str(),comlen);
						for(auto it=ldups.begin(); it!=ldups.end(); it++)
							const_cast<string&>(*it).erase(0,comlen);// !!! порядок не нарушится
					}
				}
				
				string comext;//общее расширение
				{
					auto it=ldups.begin();
					comext=find_ext(&*it++);
					for(;it!=ldups.end(); it++)
						if(comext!=find_ext(&*it))
							comext.clear();
				}
				dups[compath][comext].insert(move(make_pair(size,move(ldups))));
				
				if(it==phps->end())	break;
				size= it->second.second;
				ldups.clear();
			}
			
			tot_size+=it->second.second;
			ldups.insert(move(it->second.first));
		}
	}
	cerr<<"итого "<<bytes(tot_size)<<" избыточного"<<endl;

	//=== вывести по стартовым директориям и расширениям ===
	for(auto it_compath=dups.begin(); it_compath!=dups.end(); it_compath++){
		if(!it_compath->first.empty())
			cout 		<<"cd \""<<quote_out(it_compath->first.c_str())<<"\"";
		long long size=0;
		for(auto it_comext=it_compath->second.begin(); it_comext!=it_compath->second.end(); it_comext++)
			for(auto it_size=it_comext->second.begin(); it_size!=it_comext->second.end(); it_size++)
				size+=it_size->first*(it_size->second.size()-1);
			cout << " #"<<bytes(size);
		for(auto it_comext=it_compath->second.begin(); it_comext!=it_compath->second.end(); it_comext++){
			long long size=0;
			for(auto it_size=it_comext->second.begin(); it_size!=it_comext->second.end(); it_size++)
				size+=it_size->first*(it_size->second.size()-1);
			if(it_comext->first.size()>0)
				cout<<endl<<"#    "<< it_comext->first<<" ("<<bytes(size)<<")"<<endl;
			else
				cout<<endl<<"#    no ext"<<" ("<<bytes(size)<<")"<<endl;
			for(auto it_size=it_comext->second.begin(); it_size!=it_comext->second.end(); it_size++){
				cout	<<"#      "<<it_size->first<<endl;
				for(auto it_path=it_size->second.begin(); it_path!=it_size->second.end(); it_path++)
					cout<<"#rm \"" <<quote_out(it_path->c_str())<<"\""<<endl;
			}
		}
		if(!it_compath->first.empty())
			cout 		<<"cd -"<<endl<<endl;
	}
	cout <<"lsdups # после выполнения этого скрипта снова найти пустые файлы и пересоздать этот скрипт"<<endl;
}

//=======================================================================================
struct reverse_less{
	bool operator()(const vector<string> & l, const vector<string> & r){
		return std::lexicographical_compare(l.crbegin(),l.crend(),r.crbegin(),r.crend());
	}
};
//gcc-шная библиотека совершенно напрасно определяет операторы сравнения для векторов, сетов и наверно еще много чего
struct GF{
	long long size;
	map<vector<string>,bool,reverse_less> paths;
};
typedef map<string,GF> GF_cont;//по хешу

//преобразует строку в вектор строк
vector<string> s2vs(const char * str){
		vector<string> name;
		const char * a, * b;
		a = str;
		while((b=strchr(a,'/'))!=0){
			name.push_back(string(a,b));
			a = b+1;
		}
		name.push_back(a);
		return move(name);
}
//=== сформировать ГФы ===
void make_GFs(GF_cont * pGFs, const hps_t * phps){
	for(auto it=phps->cbegin(); it!=phps->cend(); it++){
		(*pGFs)[it->first].paths.insert(make_pair(s2vs(it->second.first.c_str()),false));
		(*pGFs)[it->first].size = it->second.second;
	}
}
//=== ГФ, size=0 ===
void make_GF0(GF * pGF0, GF_cont * pGFs){
	auto it0 = pGFs->begin();
	for(; it0!=pGFs->end(); it0++)
		if(it0->second.size==0)
			break;
	if(it0!=pGFs->end() && it0->second.size==0){
		pGF0->size=0;
		pGF0->paths=move(it0->second.paths);
		pGFs->erase(it0);
	}
	else
		pGF0->size=-1;
}

struct size_less{
	bool operator()(const vector<string> & l, const vector<string> & r)const{
		if(l.size()!=r.size())
			return l.size()<r.size();
		else
			return std::lexicographical_compare(l.cbegin(),l.cend(),r.cbegin(),r.cend());
	}
};
typedef map<set<vector<string>,size_less>,map<vector<string>,GF_cont::const_iterator,size_less>> GGP_t;

// === сформировать ГПы ===
//требуется reverse_less в map-е в GF
//true, если пары не использованы (bool==false) и последние n имен совпадают
bool end_eq_n(const pair<vector<string>,bool> & l, const pair<vector<string>,bool> & r, int n){
	//cout <<"compare_"<<n<<" - "<<l.second<<r.second<<endl;
	//cout <<l.first <<endl<<r.first<<endl;
	if(l.second || r.second)
		return false;
	auto itn1=l.first.crbegin(), itn2=r.first.crbegin();
	for(int i=0; itn1!=l.first.crend() && itn2!=r.first.crend() && i<n; itn1++, itn2++, i++)//
		if(*itn1!=*itn2)
			return false;
	//cout <<"true"<<endl;
	return true;
}
//возвращает скопированными первые size()-n имен (различающийся путь)
vector<string> my_vithout_tail(const vector<string> & src, int n){//tail_path
	vector<string> q;
	for(size_t i=0; i<src.size()-n; i++)
		q.push_back(src[i]);
	return move(q);
}
//возвращает скопированными последние n имен (совпадающий путь)
vector<string> my_tail(const vector<string> & src, int n){//vithout_tail_path
	vector<string> q;
	for(size_t i = src.size()-n; i<src.size(); i++)
		q.push_back(src[i]);
	return move(q);
}
//сформировать ГПы
void make_GPs(GGP_t * pGPs, GF_cont * pGFs){
	for(auto itgf=pGFs->begin(); itgf!=pGFs->end(); itgf++){//по всем группам файлов
		//вывод ГФ-ов
		//cout <<itgf->second.size <<"===================================="<<endl;
		//for(auto it2=itgf->second.paths.cbegin(); it2!=itgf->second.paths.cend(); it2++)
		//	cout <<it2->first <<endl;
		int max_nest_eq=0;
		auto itf1=itgf->second.paths.cbegin(), itf2=itgf->second.paths.cbegin();
		std::advance(itf2,1);//гарантировано, что элементов по крайней мере 2
		for(; itf2!=itgf->second.paths.cend(); itf1++, itf2++){//по всем файлам в группе
			auto itn1=itf1->first.crbegin(), itn2=itf2->first.crbegin();
			for(int i=0; itn1!=itf1->first.crend() && itn2!=itf2->first.crend() && i<max_nest_eq; itn1++, itn2++, i++)//
				if(*itn1!=*itn2)
					goto break_continue;
			for(; itn1!=itf1->first.crend() && itn2!=itf2->first.crend() && *itn1==*itn2; itn1++, itn2++)//
				max_nest_eq++;
		break_continue:
			;
		}
		//вычислили максимальное количество одинаковых имен с конца
		for(;max_nest_eq>0; max_nest_eq--){
			//cout << "max_nest_eq = " << max_nest_eq <<endl;

			using namespace std::placeholders;
			auto first = itgf->second.paths.begin();
			auto last = itgf->second.paths.end();
			while((first = adjacent_find(first,last,bind(end_eq_n,_1,_2,max_nest_eq)))!=last){
				auto next = first;
				typename GGP_t::key_type 
					GP_param;
				GP_param.insert( my_vithout_tail(next->first,max_nest_eq) );
				for(next++; next!=last && end_eq_n(*first,*next,max_nest_eq); next++){
					GP_param.insert( my_vithout_tail(next->first,max_nest_eq) );
					next ->second = true;
				}
				first ->second = true;
				
				//for(auto it = GP_param.begin(); it!= GP_param.end(); it++)
				//	cout<<*it<<endl;
				//cout <<"===>/"<<my_tail(first->first,max_nest_eq)<<endl;
				
				(*pGPs) [move(GP_param)] [my_tail(first->first,max_nest_eq)] = itgf;

				first=next;
			}
		}
	}
}
// === расформировать GF.size==0 ===
//вспомогательная для next_hash
void inc_s(string * ps, size_t n){
	if(n == ps->size())
		(*ps)+='.';
	else
		if( (*ps)[n]==127 ){
			(*ps)[n]='.';
			inc_s(ps,n+1);
		}
		else
			(*ps)[n]++;
}
//следующая строка (для перебора всех возможных строк)
void next_hash(string * ps){
	inc_s(ps,0);
}
//расформировать GF.size==0
void kill_GF0(GGP_t * pGPs, GF_cont * pGFs, GF * pGF0){
	int max_nest_eq=0;
	auto itf1=pGF0->paths.begin(), itf2=pGF0->paths.begin();
	std::advance(itf2,1);//гарантировано, что элементов по крайней мере 2
	for(; itf2!=pGF0->paths.end(); itf1++, itf2++){//по всем файлам в группе
		auto itn1=itf1->first.rbegin(), itn2=itf2->first.rbegin();
		for(int i=0; itn1!=itf1->first.rend() && itn2!=itf2->first.rend() && i<max_nest_eq; itn1++, itn2++, i++)//
			if(*itn1!=*itn2)
				goto break_continue2;
		for(; itn1!=itf1->first.rend() && itn2!=itf2->first.rend() && *itn1==*itn2; itn1++, itn2++)//
			max_nest_eq++;
	break_continue2:
		;
	}
	//вычислили максимальное количество одинаковых имен с конца
	string newhash=".";
	for(;max_nest_eq>0; max_nest_eq--){
		//cout << "max_nest_eq = " << max_nest_eq <<endl;

		using namespace std::placeholders;
		auto first = pGF0->paths.begin();
		auto last = pGF0->paths.end();
		while((first = adjacent_find(first,last,bind(end_eq_n,_1,_2,max_nest_eq)))!=last){
			auto next = first;
			typename GGP_t::key_type 
				GP_param;
			GP_param.insert( my_vithout_tail(next->first,max_nest_eq) );
			for(next++; next!=last && end_eq_n(*first,*next,max_nest_eq); next++){
				GP_param.insert( my_vithout_tail(next->first,max_nest_eq) );
				next ->second = true;
			}
			first ->second = true;
			
			//for(auto it = GP_param.begin(); it!= GP_param.end(); it++)
			//	cout<<*it<<endl;
			//cout <<"===>/"<<my_tail(first->first,max_nest_eq)<<endl;
			
			auto target = pGPs->find(move(GP_param));
			if(target == pGPs->end()){
				//если такой конфигурации нет - вернуть ее обратно
				for(auto it=first; it!=next; it++)
					it->second = false;
			}
			else{
				GF ngf;//создать отдельный GF
				ngf.size=0;
				for(auto it=first; it!=next; it++)
					ngf.paths.insert(*it);
				auto source = pGFs->insert(make_pair(newhash,move(ngf))).first;//добавить его к остальным GFs-ам
				next_hash(&newhash);
				//и добавить на него ссылку в GPs
				target->second[my_tail(source->second.paths.begin()->first,max_nest_eq)] = source;
			}
			
			first=next;
		}
	}
	//удалить все вырезанные пути из GF0
	for(auto it = pGF0->paths.begin(); it!=pGF0->paths.end();)
		if(it->second){
			auto itt = it;
			it++;
			pGF0->paths.erase(itt);
		}
		else
			it++;
	if(pGF0->paths.size()!=0)
		//если остаток не пуст - обрабатывать его вместе со всеми GFs
		pGFs->insert(make_pair(newhash,move(*pGF0)));
	
}

template <class it_t>
bool operator<(const it_t & l, const it_t & r){
	return l->first < r->first;
}
typedef map<vector<string>,pair<set<GGP_t>,set<GF_cont::const_iterator>>> GGPsGFs_t;

//=== сформировать ГГПы и разложить их по стартовым папкам ===
//вырезает элемент у правого и вставляет его в левого
template<class mapset, class it_t>
void mapset_splice(mapset * to, mapset * from, const it_t & it){
	to->insert(*it);//как жаль, что нет библиотечной функции, делающей move
	from->erase(it);
}
template<class MapSet>
set<typename MapSet::mapped_type> discard_first(const MapSet & from){
	set<typename MapSet::mapped_type> to;
	for(auto it=from.cbegin(); it!=from.cend(); it++)
		to.insert(it->second);
	return move(to);
}
//копирует первые n строк
vector<string> my_head(const vector<string> & from, int n){
	vector<string> to;
	auto it = from.begin();
	for(int i=0; i<n; i++, it++)
		to.push_back(*it);
	return move(to);
}
//сформировать ГГПы и разложить их по стартовым папкам
void make_GGPs(GGPsGFs_t * pGGPs, GGP_t * pGPs){
	while(!pGPs->empty()){
		GGP_t GGP;
		mapset_splice(&GGP,pGPs,pGPs->begin());
		auto union_folders = GGP.begin()->first;//copy, (set<vector<string>,size_less>)
		decltype(typename GGPsGFs_t::mapped_type().second) union_GFps = discard_first(GGP.begin()->second);
		bool changed;
		do{
			changed=false;
			for(auto itgp = pGPs->begin(); itgp!=pGPs->end(); ){
				decltype(union_folders) intersection_folders;
				std::set_intersection(union_folders.begin(),union_folders.end(),itgp->first.begin(),itgp->first.end(),
					inserter(intersection_folders,intersection_folders.end()));
				decltype(union_GFps) from_GFps = discard_first(itgp->second);
				decltype(union_GFps) intersection_GFps;
				std::set_intersection(union_GFps.begin(),union_GFps.end(),from_GFps.begin(),from_GFps.end(),
					inserter(intersection_GFps,intersection_GFps.end()));
				if(intersection_folders.empty() && intersection_GFps.empty())
					itgp++;
				else{
					#if 0
						using namespace io_util;
						cout << "проверяем пересечения union_folders" <<endl;
						for(typename set<vector<string>,size_less>::const_iterator it = union_folders.cbegin(); 
								it!=union_folders.cend(); it++){
							const vector<string> & xx = *it;
							cout<<xx<<endl;
						}
						cout<<"-P-"<<endl;
						for(auto it = itgp->first.begin(); it!=itgp->first.end(); it++)
							cout<<*it<<endl;
						cout<<"-EQ-"<<endl;
						for(auto it = intersection_folders.begin(); it!=intersection_folders.end(); it++)
							cout<<*it<<endl;
						cout << "проверяем пересечения union_GFps" <<endl;
						for(typename set<GF_cont::const_iterator>::const_iterator it = union_GFps.cbegin(); 
								it!=union_GFps.cend(); it++)
							cout<<(*it)->first<<endl;
						cout<<"-P-"<<endl;
						for(auto it = from_GFps.begin(); it!=from_GFps.end(); it++)
							cout<<(*it)->first<<endl;
						cout<<"-EQ-"<<endl;
						for(auto it = intersection_GFps.begin(); it!=intersection_GFps.end(); it++)
							cout<<(*it)->first<<endl;
					#endif
						
					changed = true;
					for(auto it = itgp->first.begin(); it!=itgp->first.end(); it++)
						union_folders.insert(*it);
					for(auto it = from_GFps.begin(); it!=from_GFps.end(); it++)
						union_GFps.insert(*it);
					auto itgptmp = itgp++;
					mapset_splice(&GGP,pGPs,itgptmp);
				}
			}
		}while(changed);
		//сформировали, теперь найдем общую папку
		auto itf1 = union_folders.begin(), itf2 = union_folders.begin();
		itf2++;//гарантированно их хотябы 2
		int max_eq=0;
		for(auto it1=itf1->begin(), it2=itf2->begin(); it1!=itf1->end() && it2!=itf2->end() && *it1==*it2; it1++, it2++)
			max_eq++;
		for(itf1++, itf2++; itf2!=union_folders.end(); itf1++, itf2++){
			int i=0;
			for(auto it1=itf1->begin(), it2=itf2->begin(); i<max_eq; it1++, it2++, i++)
				if(!(it1!=itf1->end() && it2!=itf2->end() && *it1==*it2)){
					max_eq=i;
					break;
				}
		}
		if(max_eq==0)	{
			(*pGGPs)[vector<string>({string(".")})].first.insert(move(GGP));
		}
		else
			(*pGGPs)[my_head(*union_folders.begin(),max_eq)].first.insert(move(GGP));
	}
}

//=== разложить ГФы по стартовым папкам ===
void select_GFs(GGPsGFs_t * pGGFs, const GF_cont & GFs){
	int gfcount=0;
	for(auto itgf = GFs.cbegin(); itgf!=GFs.cend(); itgf++){
		for(auto it = itgf->second.paths.begin(); it!=itgf->second.paths.end(); it++)
			if(it->second)
				goto break_continue;
		gfcount++;
		{
			auto itf1 = itgf->second.paths.begin(), itf2 = itgf->second.paths.begin();
			itf2++;//гарантированно их хотябы 2 (а если size==0 их хотя бы 1)
			int max_eq=0;
			if(itgf->second.size!=0 || itf2!=itgf->second.paths.end()){
				for(auto it1=itf1->first.begin(), it2=itf2->first.begin(); 
						it1!=itf1->first.end() && it2!=itf2->first.end() && *it1==*it2; 
						it1++, it2++){
					max_eq++;
				}
				for(itf1++, itf2++; itf2!=itgf->second.paths.end(); itf1++, itf2++){
					int i=0;
					for(auto it1=itf1->first.begin(), it2=itf2->first.begin(); i<max_eq; it1++, it2++, i++)
						if(!(it1!=itf1->first.end() && it2!=itf2->first.end() && *it1==*it2)){
							max_eq=i;
							break;
						}
				}
			}
			if(max_eq==0)	
				(*pGGFs)[vector<string>({string(".")})].second.insert(itgf);
			else
				(*pGGFs)[my_head(itgf->second.paths.begin()->first,max_eq)].second.insert(itgf);
		}
	break_continue:
		;
	}
	cerr<<gfcount<<" независимых групп файлов"<<endl;
}

//true, если первый аргумент заканчивается вторым
bool end_by(const vector<string> & base, const vector<string> & end){
	for(auto ite=end.crbegin(), itb=base.crbegin(); ite!=end.crend(); itb++, ite++)
		if(itb==base.crend() || *ite!=*itb)
			return false;
	return true;
}
//=== вывести все это ===
void print_GGX(const GGPsGFs_t & GGX, const sii_cont & psd){
	using namespace io_util;
	for(auto itpath=GGX.begin(); itpath!=GGX.end(); itpath++){
		cout<<"#==== \""<<(itpath->first)<<"\" ===="<<endl<<endl;
		//cerr<<"#==== \""<<(itpath->first)<<"\" ===="<<endl;
		//вывод групп папок
		for(auto itggp = itpath->second.first.begin(); itggp!=itpath->second.first.end(); itggp++){
			set<string> ggp_exts;
			//*itggp <-> GGP_t
			int ggp_size=0;
			for(auto itgp = itggp->begin(); itgp!=itggp->end(); itgp++){
				//itgp->first  <-> set<vector<string>,size_less>
				//itgp->second <-> map<vector<string>,GF_cont::iterator,size_less>
				long long gp_size=0;
				set<string> exts;
				for(auto itf = itgp->second.begin(); itf!=itgp->second.end(); itf++){
					string ext = find_ext(&*prev(itf->first.end()));
					exts.insert(ext);
					ggp_exts.insert(move(ext));
					gp_size+=itf->second->second.size;
				}
				cout<<"rmfromdir(){ # "<<itgp->second.size()<<" файлов, "<<bytes(gp_size);
					for(auto it = exts.begin(); it!= exts.end(); it++)
						cout<<' '<<*it;
				cout<<endl;

				//вывод списка файлов
				for(auto itf = itgp->second.begin(); itf!=itgp->second.end(); itf++){
					//itf->first  <-> vector<string>
					//itf->second <-> pair<string,GF>
					cout<<"\trm \"$1/"<<itf->first<<"\" "
						<<"# ("<<bytes(itf->second->second.size)<<") ";
					int gf_count=0;
					for(auto it = itf->second->second.paths.begin(); it!=itf->second->second.paths.end(); it++)
						//map<vector<string>,bool,reverse_less> paths;
						if(it->second
							|| end_by(it->first,itf->first))
							gf_count++;
					gf_count -= itgp->first.size();
					if(gf_count)
						cout<<itf->second->first<<" (ещё существует "<<gf_count<<" таких же файлов)";
					cout<<endl;
					
					for(auto it = itf->second->second.paths.begin(); it!=itf->second->second.paths.end(); it++)
						//map<vector<string>,bool,reverse_less> paths;
						if(!it->second
							&& !end_by(it->first,itf->first))
								cout <<"\t#rm -f \""<<(it->first)<<"\""<<endl;
				}
				cout <<"}"<<endl;
				ggp_size+= gp_size*(itgp->first.size()-1);
				
				//вывод списка папок
				for(auto itp = itgp->first.begin(); itp!=itgp->first.end(); itp++){
					string pathdir;
					for(auto it = itp->begin(); it!=itp->end(); it++)
						pathdir+=*it+'/';
					//cout << "pathdir="<<pathdir<<endl;
					auto file_it = lower_bound(psd.begin(),psd.end(),make_pair(pathdir,make_pair((long long) 0,0)),
						[](const pair<string,pair<long long,int>> & l, const pair<string,pair<long long,int>> & r){
							return l.first<r.first;//path
						}
					);
					auto file_it2 = file_it;
					int other_files=0;
					while(file_it!=psd.end() && file_it->first.find(pathdir)==0){
						//cout << file_it->first <<endl;
						other_files++, file_it++;
					}
					other_files -=itgp->second.size();
					
					cout <<"#rmfromdir \""<<(*itp)<<"/\""<<" # еще "<<other_files<<" файлов"<<endl;
					
					if(other_files<5){
						file_it = file_it2;
						for(;file_it!=psd.end() && file_it->first.find(pathdir)==0; file_it++){
							vector<string> file = s2vs(file_it->first.c_str());
							file.erase(file.begin(),file.begin()+itp->size());
							if(itgp->second.find(file)==itgp->second.end())
								cout<<"# "<<file_it->first<<endl;
						}
					}
				}
				cout <<endl;
			}
			cout<<"#-------- "<<itggp->size()<<" гп, [";
			for(auto itgp = itggp->begin(); itgp!=itggp->end(); itgp++)
				cout<<itgp->first.size()<<'x'<<itgp->second.size()<<' ';
			cout<<"] (";
			for(auto it = ggp_exts.begin(); it!=ggp_exts.end(); it++)
				cout<<" "<<*it;
			cout<<") "<<bytes(ggp_size)<<endl<<endl;
		}
		//вывод групп файлов
		int gf_size=0;
		for(auto ititgf = itpath->second.second.begin(); ititgf!=itpath->second.second.end(); ititgf++){
			set<string> exts;
			for(auto it = (*ititgf)->second.paths.begin(); it!=(*ititgf)->second.paths.end(); it++)
				exts.insert(string(find_ext(&*prev(it->first.end()))));
			cout<<"# "<<(*ititgf)->second.paths.size()<<" файлов, "<<bytes((*ititgf)->second.size);
				for(auto it = exts.begin(); it!= exts.end(); it++)
					cout<<' '<<*it;
			cout<<endl;
			gf_size += (*ititgf)->second.size*((*ititgf)->second.paths.size()-1);
			for(auto it = (*ititgf)->second.paths.begin(); it!=(*ititgf)->second.paths.end(); it++)
				cout<<"#rm \""<<(it->first)<<"\""<<endl;
			cout<<endl;
		}
		if(itpath->second.second.size()){
			cout<<"#-------- "<<itpath->second.second.size()<<" гф, [";
			for(auto ititgf = itpath->second.second.begin(); ititgf!=itpath->second.second.end(); ititgf++)
				cout<<(*ititgf)->second.paths.size()<<' ';
			cout<<"] "<<bytes(gf_size)<<" --------------------------------------------"<<endl<<endl;
		}
	}
}

void print_script_rmdirs(const hps_t * phps, const sii_cont & psd){
	using namespace io_util;

	//=== сформировать ГФы ===
	GF_cont GFs;
	make_GFs(&GFs,phps);
	
	long long int over_size=0;
	for(auto it=GFs.begin(); it!=GFs.end(); it++)
		over_size+=it->second.size * (it->second.paths.size()-1);
	cerr <<GFs.size()<<" групп файлов, избыточного "<<bytes(over_size)<<endl;

	//=== ГФ, size=0 ===
	GF GF0;
	make_GF0(&GF0,&GFs);

	//=== сформировать ГПы ===
	GGP_t GPs;//одна большая группа группа папок
	make_GPs(&GPs,&GFs);
	cerr<<GPs.size()<<" групп папок" <<endl;
	
	//=== расформировать GF.size==0 ===
	if(GF0.size!=-1){
		kill_GF0(&GPs,&GFs,&GF0);
	}
	//cerr<<"GF0 расформирован" <<endl;
	
#if 0
	//вывод ГП-ов
	for(auto itgp = GPs.begin(); itgp!=GPs.end(); itgp++){
		cout <<"rmfromdir(){"<<endl;
		for(auto itf = itgp->second.begin(); itf!=itgp->second.end(); itf++){
						cout <<"\trm $1/"<<itf->second<<" # ("<<bytes(itf->first->size)<<") "<<(void*)itf->first<<endl;
			for(auto it = itf->first->paths.begin(); it!=itf->first->paths.end(); it++)
				if(!end_by(it->first,itf->second))
					if(!it->second)
						cout <<"\t#rm -f "<<it->first<<endl;
		}
		cout <<"}"<<endl;
		for(auto itp = itgp->first.begin(); itp!=itgp->first.end(); itp++)
			cout <<"#rmfromdir "<<*itp<<endl;
		cout <<endl;
	}
#endif
	
	//=== расформировать ГПы из одной ГФ ===
		//пока отмена
		
	//=== сформировать ГГПы и разложить их по стартовым папкам ===
	GGPsGFs_t GGPsGFs;
	make_GGPs(&GGPsGFs,&GPs);
	
	//=== разложить ГФы по стартовым папкам ===
	cerr<<"before select_GFs"<<endl;
	select_GFs(&GGPsGFs,GFs);
	cerr<<"after select_GFs"<<endl;
	cerr <<GGPsGFs.size()<<" групп групп папок и независимых групп файлов"<<endl;
	
	//=== вывести все это ===
	print_GGX(GGPsGFs,psd);
}

#
