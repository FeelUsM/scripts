#include <iostream>
#include <fstream>
#define one_source
#include <strstrlib/strin.h>
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

//работа с экранированными и неэкранированными строками
char * quote_strcpy(char * to, const char * from){
	while(*from){
		if(*from=='"' || *from=='\\' || *from=='$')
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
	read_line(tmp,&s);
	cerr <<mes <<dump(s.c_str()) <<endl;
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

pair<const char *, const char *> str_mismatch(const char * s1, const char *s2){
	while(*s1 && *s2 && *s1==*s2)
		s1++, s2++;
	return make_pair(s1,s2);
}

const char * find_ext(const string * ps){
	int dirpos=ps->rfind('/'), extpos=ps->rfind('.');
	if(extpos>=0 && dirpos<extpos)
		return ps->c_str()+extpos;
	else
		return "";
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


typedef pair<string,pair<int,int>> sii_t;
typedef vector<sii_t> sii_cont;
typedef multimap<string,pair<string,int>> hps_t;
int read_files(const char * name_of_files, sii_cont * ppsd);
int read_hashes(const char * name_of_hashes, 
	map<decltype(sii_t().first),decltype(sii_t().second)> * ppsd2,
	hps_t * phps);
int calc_and_write_hashes(const char * name_of_hashes, 
	map<decltype(sii_t().first),decltype(sii_t().second)> * ppsd2,
	hps_t * phps);
void print_simple(const hps_t & hps);
void print_script_rmfiles(hps_t * phps);
void print_script_rmdirs(const hps_t * hps, const sii_cont & psd);

int main(int argc, const char * argv[]){
	int errcode;
	if(argc!=3)
	{	cerr << "usage: sldups-internal files-file hashes-file" << endl; return 1;	}
	const char * name_of_files=argv[1];
	const char * name_of_hashes=argv[2];
	
	//=== считываем файлы (размер дата имя) ===
	sii_cont psd;//path size date
	if(errcode=read_files(name_of_files,&psd))
		return errcode;
	if(psd.empty())	cerr<<"files empty"<<endl;
	std::sort(psd.begin(),psd.end(),
		[](const pair<string,pair<int,int>> & l, const pair<string,pair<int,int>> & r){
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
		[](const pair<string,pair<int,int>> & l, const pair<string,pair<int,int>> & r){
			return l.second.first==r.second.first;//size
		}
	);
	cerr << psd2.size()<<" файлов с одинаковым размером"<<endl;
	//=== считываем закешированные хеши (хеш дата имя) ===
	hps_t hps;//hash path size
	if(errcode=read_hashes(name_of_hashes,&psd2,&hps))
		return errcode;

	//for(auto it=psd2.begin(); it!=psd2.end(); it++)
		//cerr<<it->second.first<<"\t"<<it->first<<endl;
	//=== вычисляем НЕзакешированные хеши ===
	if(!psd2.empty()){
		cerr <<"у "<<psd2.size()<<" файлов будут посчитаны хеши"<<endl;
		if(errcode=calc_and_write_hashes(name_of_hashes,&psd2,&hps))
			return errcode;
	}
	//for(auto it=hps.begin(); it!=hps.end(); it++)
	//	cout <<it->first<<'\t'<<it->second.second<<'\t'<<it->second.first<<endl;
	antiuniq(&hps,[](const pair<string,pair<string,int>> & l, const pair<string,pair<string,int>> & r)
		{	return l.first==r.first;	});
	cerr <<hps.size()<<" файлов c одинаковым MD5"<<endl;
	
	//print_simple(hps);
	//print_script_rmfiles(&hps);
	print_script_rmdirs(&hps,psd);
}

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
	while(!ppsd2->empty())
	{
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
			//cerr<<it->first<<endl;
			*begin++='"';
			*begin='\0';
			it++;
		}
		//cerr<<"---"<<endl;
		forward_adressed_stream md5(true,new file_on_FILE(popen(array,"r")));
		if(!md5)	{	cerr<<"не смог вызвать openssl"<<endl;	return 1;	}
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
				if(ppsd2->size()%100==0) cerr<<"осталось "<<ppsd2->size()/100<<endl;
			}
			//cerr <<"после цикла"<<endl;
			if(ppsd2->begin()==it){	dump(md5.iter(),"не понятно, что посчитал openssl md5:\n");	return 1;	}
			hashes<<hash<<'\t'<<ppsd2->begin()->second.second<<'\t'<<ppsd2->begin()->first<<endl;
			phps->insert(move(make_pair(move(hash),make_pair(move(ppsd2->begin()->first),ppsd2->begin()->second.first))));
			ppsd2->erase(ppsd2->begin());
			if(ppsd2->size()%300==0) cerr<<"осталось "<<ppsd2->size()<<endl;
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

void print_simple(const hps_t & hps){
	//=== вывести простым способом ===
	auto sss=hps.cbegin();
	int size=0;
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

void print_script_rmfiles(hps_t * phps){
	map<string,map<string,multimap<int,set<string>>>> dups;//comm_path,ext,size,diff_path
	int tot_size=0;
	//=== сгруппировать по стартовым директориям и расширениям ===
																		//cerr<<"///"<<phps->begin()->second.second<<endl;
																		//cerr<<"итого "<<bytes(tot_size)<<" избыточного"<<endl;
	if(phps->begin()!=phps->end()){
		auto sss=phps->begin();
		int size= (phps->begin()==phps->end()) ? 0 : phps->begin()->second.second;
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
		int size=0;
		for(auto it_comext=it_compath->second.begin(); it_comext!=it_compath->second.end(); it_comext++)
			for(auto it_size=it_comext->second.begin(); it_size!=it_comext->second.end(); it_size++)
				size+=it_size->first*(it_size->second.size()-1);
			cout << " #"<<bytes(size);
		for(auto it_comext=it_compath->second.begin(); it_comext!=it_compath->second.end(); it_comext++){
			int size=0;
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

std::ostream & operator<<(std::ostream & str, const vector<string> & names){
	auto it=names.cbegin();
	if(it!=names.cend())
		str << *it++;
	for(;it!=names.cend(); it++)
		str << '/'<<*it;
	return str;
}

struct my_less{
	bool operator()(const vector<string> & l, const vector<string> & r){
		return std::lexicographical_compare(l.crbegin(),l.crend(),r.crbegin(),r.crend());
	}
	bool operator()(const set<vector<string>> & l, const set<vector<string>> & r){
		return std::lexicographical_compare(l.cbegin(),l.cend(),r.cbegin(),r.cend());
	}
};
//gcc-шная библиотека совершенно напрасно определяет операторы сравнения для векторов, сетов и наверно еще много чего

bool end_eq_n(const pair<vector<string>,bool> & l, const pair<vector<string>,bool> & r, int n){
	//cout <<"compare_"<<n<<" - "<<l.second<<r.second<<endl;
	//cout <<l.first <<endl<<r.first<<endl;
	if(l.second || r.second)
		return false;
	auto itn1=l.first.rbegin(), itn2=r.first.rbegin();
	for(int i=0; itn1!=l.first.rend() && itn2!=r.first.rend() && i<n; itn1++, itn2++, i++)//
		if(*itn1!=*itn2)
			return false;
	//cout <<"true"<<endl;
	return true;
}
vector<string> diff_path(const vector<string> & src, int n){
	vector<string> q;
	for(size_t i=0; i<src.size()-n; i++)
		q.push_back(src[i]);
	return move(q);
}
vector<string> eq_path(const vector<string> & src, int n){
	vector<string> q;
	for(size_t i = src.size()-n; i<src.size(); i++)
		q.push_back(src[i]);
	return move(q);
}
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
void next_hash(string * ps){
	inc_s(ps,0);
}
template<class mapset, class it_t>
void mapset_splice(mapset * to, mapset * from, const it_t & it){
	to->insert(*it);//как жаль, что нет библиотечной функции, делающей move
	from->erase(it);
}
template<class First, class Second>
set<First> discard_second(const set<pair<First,Second>> & from){
	set<First> to;
	for(auto it=from.cbegin(); it!=from.cend(); it++)
		to.insert(it->first);
	return move(to);
}
bool end_by(const vector<string> & base, const vector<string> & end){
	for(auto ite=end.crbegin(), itb=base.crbegin(); ite!=end.crend(); itb++, ite++)
		if(itb==base.crend() || *ite!=*itb)
			return false;
	return true;
}
void print_script_rmdirs(const hps_t * phps, const sii_cont & psd){
	//=== сформировать ГФы ===
	struct GF{
		map<vector<string>,bool,my_less> paths;
		int size;
	};
	map<string,GF> GFs;//по хешу
	
	for(auto it=phps->cbegin(); it!=phps->cend(); it++){
		vector<string> name;
		const char * a, * b;
		a = it->second.first.c_str();
		while((b=strchr(a,'/'))!=0){
			name.push_back(string(a,b));
			a = b+1;
		}
		name.push_back(a);
		GFs[it->first].paths.insert(move( make_pair(move(name),false) ));
		GFs[it->first].size=it->second.second;
	}
	long long int over_size=0;
	for(auto it=GFs.begin(); it!=GFs.end(); it++)
		over_size+=it->second.size * (it->second.paths.size()-1);
	cerr <<GFs.size()<<" групп файлов, избыточного "<<bytes(over_size)<<endl;

	//=== ГФ, size=0 ===
	GF GF0;
	auto it0 = GFs.begin();
	for(; it0!=GFs.end(); it0++)
		if(it0->second.size==0)
			break;
	if(it0!=GFs.end() && it0->second.size==0){
		GF0.size=0;
		GF0.paths=move(it0->second.paths);
		GFs.erase(it0);
	}
	else
		GF0.size=-1;
	//cerr <<"ГФ, size=0 сформирован"<<endl;

	//=== сформировать ГПы ===
	typedef map<set<vector<string>>,set<pair<GF*,vector<string>>>> GPs_t;
	GPs_t GPs;
	for(auto itgf=GFs.begin(); itgf!=GFs.end(); itgf++){//по всем группам файлов
		//вывод ГФ-ов
		//cout <<itgf->second.size <<"===================================="<<endl;
		//for(auto it2=itgf->second.paths.begin(); it2!=itgf->second.paths.end(); it2++)
		//	cout <<it2->first <<endl;
		int max_nest_eq=0;
		auto itf1=itgf->second.paths.begin(), itf2=itgf->second.paths.begin();
		std::advance(itf2,1);//гарантировано, что элементов по крайней мере 2
		for(; itf2!=itgf->second.paths.end(); itf1++, itf2++){//по всем файлам в группе
			auto itn1=itf1->first.rbegin(), itn2=itf2->first.rbegin();
			for(int i=0; itn1!=itf1->first.rend() && itn2!=itf2->first.rend() && i<max_nest_eq; itn1++, itn2++, i++)//
				if(*itn1!=*itn2)
					goto break_continue;
			for(; itn1!=itf1->first.rend() && itn2!=itf2->first.rend() && *itn1==*itn2; itn1++, itn2++)//
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
					set<vector<string>> GP_param;
				GP_param.insert( diff_path(next->first,max_nest_eq) );
				for(next++; next!=last && end_eq_n(*first,*next,max_nest_eq); next++){
					GP_param.insert( diff_path(next->first,max_nest_eq) );
					next ->second = true;
				}
				first ->second = true;
				
				//for(auto it = GP_param.begin(); it!= GP_param.end(); it++)
				//	cout<<*it<<endl;
				//cout <<"===>/"<<eq_path(first->first,max_nest_eq)<<endl;
				
				GPs[move(GP_param)].insert( move(make_pair(&itgf->second,eq_path(first->first,max_nest_eq))) );

				first=next;
			}
		}
	}
	cerr<<GPs.size()<<" групп папок" <<endl;
	//=== расформировать GF.size==0 ===
	if(GF0.size!=-1){
		int max_nest_eq=0;
		auto itf1=GF0.paths.begin(), itf2=GF0.paths.begin();
		std::advance(itf2,1);//гарантировано, что элементов по крайней мере 2
		for(; itf2!=GF0.paths.end(); itf1++, itf2++){//по всем файлам в группе
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
			auto first = GF0.paths.begin();
			auto last = GF0.paths.end();
			while((first = adjacent_find(first,last,bind(end_eq_n,_1,_2,max_nest_eq)))!=last){
				auto next = first;
					set<vector<string>> GP_param;
				GP_param.insert( diff_path(next->first,max_nest_eq) );
				for(next++; next!=last && end_eq_n(*first,*next,max_nest_eq); next++){
					GP_param.insert( diff_path(next->first,max_nest_eq) );
					next ->second = true;
				}
				first ->second = true;
				
				//for(auto it = GP_param.begin(); it!= GP_param.end(); it++)
				//	cout<<*it<<endl;
				//cout <<"===>/"<<eq_path(first->first,max_nest_eq)<<endl;
				
				auto target = GPs.find(move(GP_param));
				if(target == GPs.end()){
					//если такой конфигурации нет - вернуть ее обратно
					for(auto it=first; it!=next; it++)
						it->second = false;
				}
				else{
					GF ngf;//создать отдельный GF
					ngf.size=0;
					for(auto it=first; it!=next; it++)
						ngf.paths.insert(*it);
					GF * pgf = &GFs.insert(make_pair(newhash,move(ngf))).first->second;//добавить его к остальным GFs-ам
					next_hash(&newhash);
					//и добавить на него ссылку в GPs
					target->second.insert( move(make_pair(pgf,eq_path(pgf->paths.begin()->first,max_nest_eq))) );
				}
				
				first=next;
			}
		}
		//удалить все вырезанные пути из GF0
		for(auto it = GF0.paths.begin(); it!=GF0.paths.end();)
			if(it->second){
				auto itt = it;
				it++;
				GF0.paths.erase(itt);
			}
			else
				it++;
		if(GF0.paths.size()!=0)
			//если остаток не пуст - обрабатывать его вместе со всеми GFs
			GFs.insert(make_pair(newhash,move(GF0)));
	}
	
	//cerr<<"GF0 расформирован" <<endl;
	if(0)//вывод ГП-ов
	for(auto itgp = GPs.begin(); itgp!=GPs.end(); itgp++){
		cout <<"rmfromdir(){"<<endl;
		for(auto itf = itgp->second.begin(); itf!=itgp->second.end(); itf++){
			cout <<"\trm $1/"<<itf->second<<" # ("<<itf->first->size<<") "<<(void*)itf->first<<endl;
			for(auto it = itf->first->paths.begin(); it!=itf->first->paths.end(); it++)
				if(!end_by(it->first,itf->second))
					if(it->second)
						cout <<"\t# "<<it->first<<endl;
					else
						cout <<"\t#rm -f "<<it->first<<endl;
		}
		cout <<"}"<<endl;
		for(auto itp = itgp->first.begin(); itp!=itgp->first.end(); itp++)
			cout <<"#rmfromdir "<<*itp<<endl;
		cout <<endl;
	}
	
	//=== расформировать ГПы из одной ГФ ===
		//пока отмена
	//=== сформировать ГГПы и разложить их по стартовым папкам ===
	map<string,pair<set<GPs_t>,set<GF*>>> GGPsGFs;
	while(!GPs.empty()){
		GPs_t GGP;
		mapset_splice(&GGP,&GPs,GPs.begin());
		set<vector<string>> union_folders = GGP.begin()->first;
		set<GF*> union_GFps = move(discard_second(GGP.begin()->second));
		bool changed;
		do{
			changed=false;
			for(auto itgp = GPs.begin(); itgp!=GPs.end(); ){
				set<vector<string>> intersection_folders;
				std::set_intersection(union_folders.begin(),union_folders.end(),itgp->first.begin(),itgp->first.end(),
					inserter(intersection_folders,intersection_folders.end()));
				set<GF*> from_GFps = move(discard_second(itgp->second));
				set<GF*> intersection_GFps;
				std::set_intersection(union_GFps.begin(),union_GFps.end(),from_GFps.begin(),from_GFps.end(),
					inserter(intersection_GFps,intersection_GFps.end()));
				if(intersection_folders.empty() && intersection_GFps.empty())
					itgp++;
				else{
					#if 0
					cout << "проверяем пересечения union_folders" <<endl;
					for(auto it = union_folders.begin(); it!=union_folders.end(); it++)
						cout<<*it<<endl;
					cout<<"-P-"<<endl;
					for(auto it = itgp->first.begin(); it!=itgp->first.end(); it++)
						cout<<*it<<endl;
					cout<<"-EQ-"<<endl;
					for(auto it = intersection_folders.begin(); it!=intersection_folders.end(); it++)
						cout<<*it<<endl;
					cout << "проверяем пересечения union_GFps" <<endl;
					for(auto it = union_GFps.begin(); it!=union_GFps.end(); it++)
						cout<<(void*)*it<<endl;
					cout<<"-P-"<<endl;
					for(auto it = from_GFps.begin(); it!=from_GFps.end(); it++)
						cout<<(void*)*it<<endl;
					cout<<"-EQ-"<<endl;
					for(auto it = intersection_GFps.begin(); it!=intersection_GFps.end(); it++)
						cout<<(void*)*it<<endl;
					#endif
						
					changed = true;
					for(auto it = itgp->first.begin(); it!=itgp->first.end(); it++)
						union_folders.insert(*it);
					for(auto it = from_GFps.begin(); it!=from_GFps.end(); it++)
						union_GFps.insert(*it);
					auto itgptmp = itgp++;
					mapset_splice(&GGP,&GPs,itgptmp);
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
		if(max_eq==0)	throw "max_eq==0";
			
		auto it=union_folders.begin()->begin();
		string path = *it++;
		for(int i=1; i<max_eq; i++)
			path+='/'+*it++;
		GGPsGFs[path].first.insert(move(GGP));
	}
	//cerr <<GGPsGFs.size()<<" ГГП-оп после добавления ГП-ов"<<endl;
	//for(auto it = GGPsGFs.begin(); it!=GGPsGFs.end(); it++)
		//cerr<<it->first<<endl;
	
	//=== разложить ГФы по стартовым папкам ===
	int gfcount=0;
	for(auto itgf = GFs.begin(); itgf!=GFs.end(); itgf++){
		for(auto it = itgf->second.paths.begin(); it!=itgf->second.paths.end(); it++)
			if(it->second)
				goto break_continue3;
		gfcount++;
		{
			auto itf1 = itgf->second.paths.begin(), itf2 = itgf->second.paths.begin();
			itf2++;//гарантированно их хотябы 2
			int max_eq=0;
			for(auto it1=itf1->first.begin(), it2=itf2->first.begin(); it1!=itf1->first.end() && it2!=itf2->first.end() && *it1==*it2; 
				it1++, it2++)
				max_eq++;
			for(itf1++, itf2++; itf2!=itgf->second.paths.end(); itf1++, itf2++){
				int i=0;
				for(auto it1=itf1->first.begin(), it2=itf2->first.begin(); i<max_eq; it1++, it2++, i++)
					if(!(it1!=itf1->first.end() && it2!=itf2->first.end() && *it1==*it2)){
						max_eq=i;
						break;
					}
			}
			if(max_eq==0)	throw "max_eq==0";
				
			auto it=itgf->second.paths.begin()->first.begin();
			string path = *it++;
			for(int i=1; i<max_eq; i++)
				path+='/'+*it++;
			GGPsGFs[path].second.insert(&itgf->second);
		}
	break_continue3:
		;
	}
	cerr<<gfcount<<" независимых групп файлов"<<endl;
	cerr <<GGPsGFs.size()<<" групп групп папок и независимых групп файлов"<<endl;
	//for(auto it = GGPsGFs.begin(); it!=GGPsGFs.end(); it++)
		//cerr<<it->first<<endl;
	
	//=== вывести все это ===
	
}

#
