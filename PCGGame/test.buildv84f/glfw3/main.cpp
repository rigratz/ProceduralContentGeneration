
#include "main.h"

//${CONFIG_BEGIN}
#define CFG_BINARY_FILES *.bin|*.dat
#define CFG_BRL_GAMETARGET_IMPLEMENTED 1
#define CFG_BRL_THREAD_IMPLEMENTED 1
#define CFG_CD 
#define CFG_CONFIG debug
#define CFG_CPP_GC_MODE 1
#define CFG_GLFW_SWAP_INTERVAL -1
#define CFG_GLFW_USE_MINGW 1
#define CFG_GLFW_VERSION 3
#define CFG_GLFW_WINDOW_DECORATED 1
#define CFG_GLFW_WINDOW_FLOATING 0
#define CFG_GLFW_WINDOW_FULLSCREEN 0
#define CFG_GLFW_WINDOW_HEIGHT 480
#define CFG_GLFW_WINDOW_RESIZABLE 0
#define CFG_GLFW_WINDOW_SAMPLES 0
#define CFG_GLFW_WINDOW_TITLE Monkey Game
#define CFG_GLFW_WINDOW_WIDTH 640
#define CFG_HOST macos
#define CFG_IMAGE_FILES *.png|*.jpg
#define CFG_LANG cpp
#define CFG_MODPATH 
#define CFG_MOJO_AUTO_SUSPEND_ENABLED 1
#define CFG_MOJO_DRIVER_IMPLEMENTED 1
#define CFG_MOJO_IMAGE_FILTERING_ENABLED 1
#define CFG_MUSIC_FILES *.wav|*.ogg
#define CFG_OPENGL_DEPTH_BUFFER_ENABLED 0
#define CFG_OPENGL_GLES20_ENABLED 0
#define CFG_SAFEMODE 0
#define CFG_SOUND_FILES *.wav|*.ogg
#define CFG_TARGET glfw
#define CFG_TEXT_FILES *.txt|*.xml|*.json
//${CONFIG_END}

//${TRANSCODE_BEGIN}

#include <wctype.h>
#include <locale.h>

// C++ Monkey runtime.
//
// Placed into the public domain 24/02/2011.
// No warranty implied; use at your own risk.

//***** Monkey Types *****

typedef wchar_t Char;
template<class T> class Array;
class String;
class Object;

#if CFG_CPP_DOUBLE_PRECISION_FLOATS
typedef double Float;
#define FLOAT(X) X
#else
typedef float Float;
#define FLOAT(X) X##f
#endif

void dbg_error( const char *p );

#if !_MSC_VER
#define sprintf_s sprintf
#define sscanf_s sscanf
#endif

//***** GC Config *****

#if CFG_CPP_GC_DEBUG
#define DEBUG_GC 1
#else
#define DEBUG_GC 0
#endif

// GC mode:
//
// 0 = disabled
// 1 = Incremental GC every OnWhatever
// 2 = Incremental GC every allocation
//
#ifndef CFG_CPP_GC_MODE
#define CFG_CPP_GC_MODE 1
#endif

//How many bytes alloced to trigger GC
//
#ifndef CFG_CPP_GC_TRIGGER
#define CFG_CPP_GC_TRIGGER 8*1024*1024
#endif

//GC_MODE 2 needs to track locals on a stack - this may need to be bumped if your app uses a LOT of locals, eg: is heavily recursive...
//
#ifndef CFG_CPP_GC_MAX_LOCALS
#define CFG_CPP_GC_MAX_LOCALS 8192
#endif

// ***** GC *****

#if _WIN32

int gc_micros(){
	static int f;
	static LARGE_INTEGER pcf;
	if( !f ){
		if( QueryPerformanceFrequency( &pcf ) && pcf.QuadPart>=1000000L ){
			pcf.QuadPart/=1000000L;
			f=1;
		}else{
			f=-1;
		}
	}
	if( f>0 ){
		LARGE_INTEGER pc;
		if( QueryPerformanceCounter( &pc ) ) return pc.QuadPart/pcf.QuadPart;
		f=-1;
	}
	return 0;// timeGetTime()*1000;
}

#elif __APPLE__

#include <mach/mach_time.h>

int gc_micros(){
	static int f;
	static mach_timebase_info_data_t timeInfo;
	if( !f ){
		mach_timebase_info( &timeInfo );
		timeInfo.denom*=1000L;
		f=1;
	}
	return mach_absolute_time()*timeInfo.numer/timeInfo.denom;
}

#else

int gc_micros(){
	return 0;
}

#endif

#define gc_mark_roots gc_mark

void gc_mark_roots();

struct gc_object;

gc_object *gc_object_alloc( int size );
void gc_object_free( gc_object *p );

struct gc_object{
	gc_object *succ;
	gc_object *pred;
	int flags;
	
	virtual ~gc_object(){
	}
	
	virtual void mark(){
	}
	
	void *operator new( size_t size ){
		return gc_object_alloc( size );
	}
	
	void operator delete( void *p ){
		gc_object_free( (gc_object*)p );
	}
};

gc_object gc_free_list;
gc_object gc_marked_list;
gc_object gc_unmarked_list;
gc_object gc_queued_list;	//doesn't really need to be doubly linked...

int gc_free_bytes;
int gc_marked_bytes;
int gc_alloced_bytes;
int gc_max_alloced_bytes;
int gc_new_bytes;
int gc_markbit=1;

gc_object *gc_cache[8];

void gc_collect_all();
void gc_mark_queued( int n );

#define GC_CLEAR_LIST( LIST ) ((LIST).succ=(LIST).pred=&(LIST))

#define GC_LIST_IS_EMPTY( LIST ) ((LIST).succ==&(LIST))

#define GC_REMOVE_NODE( NODE ){\
(NODE)->pred->succ=(NODE)->succ;\
(NODE)->succ->pred=(NODE)->pred;}

#define GC_INSERT_NODE( NODE,SUCC ){\
(NODE)->pred=(SUCC)->pred;\
(NODE)->succ=(SUCC);\
(SUCC)->pred->succ=(NODE);\
(SUCC)->pred=(NODE);}

void gc_init1(){
	GC_CLEAR_LIST( gc_free_list );
	GC_CLEAR_LIST( gc_marked_list );
	GC_CLEAR_LIST( gc_unmarked_list);
	GC_CLEAR_LIST( gc_queued_list );
}

void gc_init2(){
	gc_mark_roots();
}

#if CFG_CPP_GC_MODE==2

int gc_ctor_nest;
gc_object *gc_locals[CFG_CPP_GC_MAX_LOCALS],**gc_locals_sp=gc_locals;

struct gc_ctor{
	gc_ctor(){ ++gc_ctor_nest; }
	~gc_ctor(){ --gc_ctor_nest; }
};

struct gc_enter{
	gc_object **sp;
	gc_enter():sp(gc_locals_sp){
	}
	~gc_enter(){
#if DEBUG_GC
		static int max_locals;
		int n=gc_locals_sp-gc_locals;
		if( n>max_locals ){
			max_locals=n;
			printf( "max_locals=%i\n",n );
		}
#endif		
		gc_locals_sp=sp;
	}
};

#define GC_CTOR gc_ctor _c;
#define GC_ENTER gc_enter _e;

#else

struct gc_ctor{
};
struct gc_enter{
};

#define GC_CTOR
#define GC_ENTER

#endif

//Can be modified off thread!
static volatile int gc_ext_new_bytes;

#if _MSC_VER
#define atomic_add(P,V) InterlockedExchangeAdd((volatile unsigned int*)P,V)			//(*(P)+=(V))
#define atomic_sub(P,V) InterlockedExchangeSubtract((volatile unsigned int*)P,V)	//(*(P)-=(V))
#else
#define atomic_add(P,V) __sync_fetch_and_add(P,V)
#define atomic_sub(P,V) __sync_fetch_and_sub(P,V)
#endif

//Careful! May be called off thread!
//
void gc_ext_malloced( int size ){
	atomic_add( &gc_ext_new_bytes,size );
}

void gc_object_free( gc_object *p ){

	int size=p->flags & ~7;
	gc_free_bytes-=size;
	
	if( size<64 ){
		p->succ=gc_cache[size>>3];
		gc_cache[size>>3]=p;
	}else{
		free( p );
	}
}

void gc_flush_free( int size ){

	int t=gc_free_bytes-size;
	if( t<0 ) t=0;
	
	while( gc_free_bytes>t ){
	
		gc_object *p=gc_free_list.succ;

		GC_REMOVE_NODE( p );

#if DEBUG_GC
//		printf( "deleting @%p\n",p );fflush( stdout );
//		p->flags|=4;
//		continue;
#endif
		delete p;
	}
}

gc_object *gc_object_alloc( int size ){

	size=(size+7)&~7;
	
	gc_new_bytes+=size;
	
#if CFG_CPP_GC_MODE==2

	if( !gc_ctor_nest ){

#if DEBUG_GC
		int ms=gc_micros();
#endif
		if( gc_new_bytes+gc_ext_new_bytes>(CFG_CPP_GC_TRIGGER) ){
			atomic_sub( &gc_ext_new_bytes,gc_ext_new_bytes );
			gc_collect_all();
			gc_new_bytes=0;
		}else{
			gc_mark_queued( (long long)(gc_new_bytes)*(gc_alloced_bytes-gc_new_bytes)/(CFG_CPP_GC_TRIGGER)+gc_new_bytes );
		}

#if DEBUG_GC
		ms=gc_micros()-ms;
		if( ms>=100 ) {printf( "gc time:%i\n",ms );fflush( stdout );}
#endif
	}
	
#endif

	gc_flush_free( size );

	gc_object *p;
	if( size<64 && (p=gc_cache[size>>3]) ){
		gc_cache[size>>3]=p->succ;
	}else{
		p=(gc_object*)malloc( size );
	}
	
	p->flags=size|gc_markbit;
	GC_INSERT_NODE( p,&gc_unmarked_list );

	gc_alloced_bytes+=size;
	if( gc_alloced_bytes>gc_max_alloced_bytes ) gc_max_alloced_bytes=gc_alloced_bytes;
	
#if CFG_CPP_GC_MODE==2
	*gc_locals_sp++=p;
#endif

	return p;
}

#if DEBUG_GC

template<class T> gc_object *to_gc_object( T *t ){
	gc_object *p=dynamic_cast<gc_object*>(t);
	if( p && (p->flags & 4) ){
		printf( "gc error : object already deleted @%p\n",p );fflush( stdout );
		exit(-1);
	}
	return p;
}

#else

#define to_gc_object(t) dynamic_cast<gc_object*>(t)

#endif

template<class T> T *gc_retain( T *t ){
#if CFG_CPP_GC_MODE==2
	*gc_locals_sp++=to_gc_object( t );
#endif
	return t;
}

template<class T> void gc_mark( T *t ){

	gc_object *p=to_gc_object( t );
	
	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_marked_list );
		gc_marked_bytes+=(p->flags & ~7);
		p->mark();
	}
}

template<class T> void gc_mark_q( T *t ){

	gc_object *p=to_gc_object( t );
	
	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_queued_list );
	}
}

template<class T,class V> void gc_assign( T *&lhs,V *rhs ){

	gc_object *p=to_gc_object( rhs );
	
	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_queued_list );
	}
	lhs=rhs;
}

void gc_mark_locals(){

#if CFG_CPP_GC_MODE==2
	for( gc_object **pp=gc_locals;pp!=gc_locals_sp;++pp ){
		gc_object *p=*pp;
		if( p && (p->flags & 3)==gc_markbit ){
			p->flags^=1;
			GC_REMOVE_NODE( p );
			GC_INSERT_NODE( p,&gc_marked_list );
			gc_marked_bytes+=(p->flags & ~7);
			p->mark();
		}
	}
#endif	
}

void gc_mark_queued( int n ){
	while( gc_marked_bytes<n && !GC_LIST_IS_EMPTY( gc_queued_list ) ){
		gc_object *p=gc_queued_list.succ;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_marked_list );
		gc_marked_bytes+=(p->flags & ~7);
		p->mark();
	}
}

void gc_validate_list( gc_object &list,const char *msg ){
	gc_object *node=list.succ;
	while( node ){
		if( node==&list ) return;
		if( !node->pred ) break;
		if( node->pred->succ!=node ) break;
		node=node->succ;
	}
	if( msg ){
		puts( msg );fflush( stdout );
	}
	puts( "LIST ERROR!" );
	exit(-1);
}

//returns reclaimed bytes
void gc_sweep(){

	int reclaimed_bytes=gc_alloced_bytes-gc_marked_bytes;
	
	if( reclaimed_bytes ){
	
		//append unmarked list to end of free list
		gc_object *head=gc_unmarked_list.succ;
		gc_object *tail=gc_unmarked_list.pred;
		gc_object *succ=&gc_free_list;
		gc_object *pred=succ->pred;
		
		head->pred=pred;
		tail->succ=succ;
		pred->succ=head;
		succ->pred=tail;
		
		gc_free_bytes+=reclaimed_bytes;
	}

	//move marked to unmarked.
	if( GC_LIST_IS_EMPTY( gc_marked_list ) ){
		GC_CLEAR_LIST( gc_unmarked_list );
	}else{
		gc_unmarked_list.succ=gc_marked_list.succ;
		gc_unmarked_list.pred=gc_marked_list.pred;
		gc_unmarked_list.succ->pred=gc_unmarked_list.pred->succ=&gc_unmarked_list;
		GC_CLEAR_LIST( gc_marked_list );
	}
	
	//adjust sizes
	gc_alloced_bytes=gc_marked_bytes;
	gc_marked_bytes=0;
	gc_markbit^=1;
}

void gc_collect_all(){

//	puts( "Mark locals" );
	gc_mark_locals();

//	puts( "Marked queued" );
	gc_mark_queued( 0x7fffffff );

//	puts( "Sweep" );
	gc_sweep();

//	puts( "Mark roots" );
	gc_mark_roots();

#if DEBUG_GC
	gc_validate_list( gc_marked_list,"Validating gc_marked_list"  );
	gc_validate_list( gc_unmarked_list,"Validating gc_unmarked_list"  );
	gc_validate_list( gc_free_list,"Validating gc_free_list" );
#endif

}

void gc_collect(){
	
#if CFG_CPP_GC_MODE==1

#if DEBUG_GC
	int ms=gc_micros();
#endif

	if( gc_new_bytes+gc_ext_new_bytes>(CFG_CPP_GC_TRIGGER) ){
		atomic_sub( &gc_ext_new_bytes,gc_ext_new_bytes );
		gc_collect_all();
		gc_new_bytes=0;
	}else{
		gc_mark_queued( (long long)(gc_new_bytes)*(gc_alloced_bytes-gc_new_bytes)/(CFG_CPP_GC_TRIGGER)+gc_new_bytes );
	}

#if DEBUG_GC
	ms=gc_micros()-ms;
//	if( ms>=100 ) {printf( "gc time:%i\n",ms );fflush( stdout );}
	if( ms>10 ) {printf( "gc time:%i\n",ms );fflush( stdout );}
#endif

#endif
}

// ***** Array *****

template<class T> T *t_memcpy( T *dst,const T *src,int n ){
	memcpy( dst,src,n*sizeof(T) );
	return dst+n;
}

template<class T> T *t_memset( T *dst,int val,int n ){
	memset( dst,val,n*sizeof(T) );
	return dst+n;
}

template<class T> int t_memcmp( const T *x,const T *y,int n ){
	return memcmp( x,y,n*sizeof(T) );
}

template<class T> int t_strlen( const T *p ){
	const T *q=p++;
	while( *q++ ){}
	return q-p;
}

template<class T> T *t_create( int n,T *p ){
	t_memset( p,0,n );
	return p+n;
}

template<class T> T *t_create( int n,T *p,const T *q ){
	t_memcpy( p,q,n );
	return p+n;
}

template<class T> void t_destroy( int n,T *p ){
}

template<class T> void gc_mark_elements( int n,T *p ){
}

template<class T> void gc_mark_elements( int n,T **p ){
	for( int i=0;i<n;++i ) gc_mark( p[i] );
}

template<class T> class Array{
public:
	Array():rep( &nullRep ){
	}

	//Uses default...
//	Array( const Array<T> &t )...
	
	Array( int length ):rep( Rep::alloc( length ) ){
		t_create( rep->length,rep->data );
	}
	
	Array( const T *p,int length ):rep( Rep::alloc(length) ){
		t_create( rep->length,rep->data,p );
	}
	
	~Array(){
	}

	//Uses default...
//	Array &operator=( const Array &t )...
	
	int Length()const{ 
		return rep->length; 
	}
	
	T &At( int index ){
		if( index<0 || index>=rep->length ) dbg_error( "Array index out of range" );
		return rep->data[index]; 
	}
	
	const T &At( int index )const{
		if( index<0 || index>=rep->length ) dbg_error( "Array index out of range" );
		return rep->data[index]; 
	}
	
	T &operator[]( int index ){
		return rep->data[index]; 
	}

	const T &operator[]( int index )const{
		return rep->data[index]; 
	}
	
	Array Slice( int from,int term )const{
		int len=rep->length;
		if( from<0 ){ 
			from+=len;
			if( from<0 ) from=0;
		}else if( from>len ){
			from=len;
		}
		if( term<0 ){
			term+=len;
		}else if( term>len ){
			term=len;
		}
		if( term<=from ) return Array();
		return Array( rep->data+from,term-from );
	}

	Array Slice( int from )const{
		return Slice( from,rep->length );
	}
	
	Array Resize( int newlen )const{
		if( newlen<=0 ) return Array();
		int n=rep->length;
		if( newlen<n ) n=newlen;
		Rep *p=Rep::alloc( newlen );
		T *q=p->data;
		q=t_create( n,q,rep->data );
		q=t_create( (newlen-n),q );
		return Array( p );
	}
	
private:
	struct Rep : public gc_object{
		int length;
		T data[0];
		
		Rep():length(0){
			flags=3;
		}
		
		Rep( int length ):length(length){
		}
		
		~Rep(){
			t_destroy( length,data );
		}
		
		void mark(){
			gc_mark_elements( length,data );
		}
		
		static Rep *alloc( int length ){
			if( !length ) return &nullRep;
			void *p=gc_object_alloc( sizeof(Rep)+length*sizeof(T) );
			return ::new(p) Rep( length );
		}
		
	};
	Rep *rep;
	
	static Rep nullRep;
	
	template<class C> friend void gc_mark( Array<C> t );
	template<class C> friend void gc_mark_q( Array<C> t );
	template<class C> friend Array<C> gc_retain( Array<C> t );
	template<class C> friend void gc_assign( Array<C> &lhs,Array<C> rhs );
	template<class C> friend void gc_mark_elements( int n,Array<C> *p );
	
	Array( Rep *rep ):rep(rep){
	}
};

template<class T> typename Array<T>::Rep Array<T>::nullRep;

template<class T> Array<T> *t_create( int n,Array<T> *p ){
	for( int i=0;i<n;++i ) *p++=Array<T>();
	return p;
}

template<class T> Array<T> *t_create( int n,Array<T> *p,const Array<T> *q ){
	for( int i=0;i<n;++i ) *p++=*q++;
	return p;
}

template<class T> void gc_mark( Array<T> t ){
	gc_mark( t.rep );
}

template<class T> void gc_mark_q( Array<T> t ){
	gc_mark_q( t.rep );
}

template<class T> Array<T> gc_retain( Array<T> t ){
#if CFG_CPP_GC_MODE==2
	gc_retain( t.rep );
#endif
	return t;
}

template<class T> void gc_assign( Array<T> &lhs,Array<T> rhs ){
	gc_mark( rhs.rep );
	lhs=rhs;
}

template<class T> void gc_mark_elements( int n,Array<T> *p ){
	for( int i=0;i<n;++i ) gc_mark( p[i].rep );
}
		
// ***** String *****

static const char *_str_load_err;

class String{
public:
	String():rep( &nullRep ){
	}
	
	String( const String &t ):rep( t.rep ){
		rep->retain();
	}

	String( int n ){
		char buf[256];
		sprintf_s( buf,"%i",n );
		rep=Rep::alloc( t_strlen(buf) );
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
	}
	
	String( Float n ){
		char buf[256];
		
		//would rather use snprintf, but it's doing weird things in MingW.
		//
		sprintf_s( buf,"%.17lg",n );
		//
		char *p;
		for( p=buf;*p;++p ){
			if( *p=='.' || *p=='e' ) break;
		}
		if( !*p ){
			*p++='.';
			*p++='0';
			*p=0;
		}

		rep=Rep::alloc( t_strlen(buf) );
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
	}

	String( Char ch,int length ):rep( Rep::alloc(length) ){
		for( int i=0;i<length;++i ) rep->data[i]=ch;
	}

	String( const Char *p ):rep( Rep::alloc(t_strlen(p)) ){
		t_memcpy( rep->data,p,rep->length );
	}

	String( const Char *p,int length ):rep( Rep::alloc(length) ){
		t_memcpy( rep->data,p,rep->length );
	}
	
#if __OBJC__	
	String( NSString *nsstr ):rep( Rep::alloc([nsstr length]) ){
		unichar *buf=(unichar*)malloc( rep->length * sizeof(unichar) );
		[nsstr getCharacters:buf range:NSMakeRange(0,rep->length)];
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
		free( buf );
	}
#endif

#if __cplusplus_winrt
	String( Platform::String ^str ):rep( Rep::alloc(str->Length()) ){
		for( int i=0;i<rep->length;++i ) rep->data[i]=str->Data()[i];
	}
#endif

	~String(){
		rep->release();
	}
	
	template<class C> String( const C *p ):rep( Rep::alloc(t_strlen(p)) ){
		for( int i=0;i<rep->length;++i ) rep->data[i]=p[i];
	}
	
	template<class C> String( const C *p,int length ):rep( Rep::alloc(length) ){
		for( int i=0;i<rep->length;++i ) rep->data[i]=p[i];
	}
	
	String Copy()const{
		Rep *crep=Rep::alloc( rep->length );
		t_memcpy( crep->data,rep->data,rep->length );
		return String( crep );
	}
	
	int Length()const{
		return rep->length;
	}
	
	const Char *Data()const{
		return rep->data;
	}
	
	Char At( int index )const{
		if( index<0 || index>=rep->length ) dbg_error( "Character index out of range" );
		return rep->data[index]; 
	}
	
	Char operator[]( int index )const{
		return rep->data[index];
	}
	
	String &operator=( const String &t ){
		t.rep->retain();
		rep->release();
		rep=t.rep;
		return *this;
	}
	
	String &operator+=( const String &t ){
		return operator=( *this+t );
	}
	
	int Compare( const String &t )const{
		int n=rep->length<t.rep->length ? rep->length : t.rep->length;
		for( int i=0;i<n;++i ){
			if( int q=(int)(rep->data[i])-(int)(t.rep->data[i]) ) return q;
		}
		return rep->length-t.rep->length;
	}
	
	bool operator==( const String &t )const{
		return rep->length==t.rep->length && t_memcmp( rep->data,t.rep->data,rep->length )==0;
	}
	
	bool operator!=( const String &t )const{
		return rep->length!=t.rep->length || t_memcmp( rep->data,t.rep->data,rep->length )!=0;
	}
	
	bool operator<( const String &t )const{
		return Compare( t )<0;
	}
	
	bool operator<=( const String &t )const{
		return Compare( t )<=0;
	}
	
	bool operator>( const String &t )const{
		return Compare( t )>0;
	}
	
	bool operator>=( const String &t )const{
		return Compare( t )>=0;
	}
	
	String operator+( const String &t )const{
		if( !rep->length ) return t;
		if( !t.rep->length ) return *this;
		Rep *p=Rep::alloc( rep->length+t.rep->length );
		Char *q=p->data;
		q=t_memcpy( q,rep->data,rep->length );
		q=t_memcpy( q,t.rep->data,t.rep->length );
		return String( p );
	}
	
	int Find( String find,int start=0 )const{
		if( start<0 ) start=0;
		while( start+find.rep->length<=rep->length ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			++start;
		}
		return -1;
	}
	
	int FindLast( String find )const{
		int start=rep->length-find.rep->length;
		while( start>=0 ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			--start;
		}
		return -1;
	}
	
	int FindLast( String find,int start )const{
		if( start>rep->length-find.rep->length ) start=rep->length-find.rep->length;
		while( start>=0 ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			--start;
		}
		return -1;
	}
	
	String Trim()const{
		int i=0,i2=rep->length;
		while( i<i2 && rep->data[i]<=32 ) ++i;
		while( i2>i && rep->data[i2-1]<=32 ) --i2;
		if( i==0 && i2==rep->length ) return *this;
		return String( rep->data+i,i2-i );
	}

	Array<String> Split( String sep )const{
	
		if( !sep.rep->length ){
			Array<String> bits( rep->length );
			for( int i=0;i<rep->length;++i ){
				bits[i]=String( (Char)(*this)[i],1 );
			}
			return bits;
		}
		
		int i=0,i2,n=1;
		while( (i2=Find( sep,i ))!=-1 ){
			++n;
			i=i2+sep.rep->length;
		}
		Array<String> bits( n );
		if( n==1 ){
			bits[0]=*this;
			return bits;
		}
		i=0;n=0;
		while( (i2=Find( sep,i ))!=-1 ){
			bits[n++]=Slice( i,i2 );
			i=i2+sep.rep->length;
		}
		bits[n]=Slice( i );
		return bits;
	}

	String Join( Array<String> bits )const{
		if( bits.Length()==0 ) return String();
		if( bits.Length()==1 ) return bits[0];
		int newlen=rep->length * (bits.Length()-1);
		for( int i=0;i<bits.Length();++i ){
			newlen+=bits[i].rep->length;
		}
		Rep *p=Rep::alloc( newlen );
		Char *q=p->data;
		q=t_memcpy( q,bits[0].rep->data,bits[0].rep->length );
		for( int i=1;i<bits.Length();++i ){
			q=t_memcpy( q,rep->data,rep->length );
			q=t_memcpy( q,bits[i].rep->data,bits[i].rep->length );
		}
		return String( p );
	}

	String Replace( String find,String repl )const{
		int i=0,i2,newlen=0;
		while( (i2=Find( find,i ))!=-1 ){
			newlen+=(i2-i)+repl.rep->length;
			i=i2+find.rep->length;
		}
		if( !i ) return *this;
		newlen+=rep->length-i;
		Rep *p=Rep::alloc( newlen );
		Char *q=p->data;
		i=0;
		while( (i2=Find( find,i ))!=-1 ){
			q=t_memcpy( q,rep->data+i,i2-i );
			q=t_memcpy( q,repl.rep->data,repl.rep->length );
			i=i2+find.rep->length;
		}
		q=t_memcpy( q,rep->data+i,rep->length-i );
		return String( p );
	}

	String ToLower()const{
		for( int i=0;i<rep->length;++i ){
			Char t=towlower( rep->data[i] );
			if( t==rep->data[i] ) continue;
			Rep *p=Rep::alloc( rep->length );
			Char *q=p->data;
			t_memcpy( q,rep->data,i );
			for( q[i++]=t;i<rep->length;++i ){
				q[i]=towlower( rep->data[i] );
			}
			return String( p );
		}
		return *this;
	}

	String ToUpper()const{
		for( int i=0;i<rep->length;++i ){
			Char t=towupper( rep->data[i] );
			if( t==rep->data[i] ) continue;
			Rep *p=Rep::alloc( rep->length );
			Char *q=p->data;
			t_memcpy( q,rep->data,i );
			for( q[i++]=t;i<rep->length;++i ){
				q[i]=towupper( rep->data[i] );
			}
			return String( p );
		}
		return *this;
	}
	
	bool Contains( String sub )const{
		return Find( sub )!=-1;
	}

	bool StartsWith( String sub )const{
		return sub.rep->length<=rep->length && !t_memcmp( rep->data,sub.rep->data,sub.rep->length );
	}

	bool EndsWith( String sub )const{
		return sub.rep->length<=rep->length && !t_memcmp( rep->data+rep->length-sub.rep->length,sub.rep->data,sub.rep->length );
	}
	
	String Slice( int from,int term )const{
		int len=rep->length;
		if( from<0 ){
			from+=len;
			if( from<0 ) from=0;
		}else if( from>len ){
			from=len;
		}
		if( term<0 ){
			term+=len;
		}else if( term>len ){
			term=len;
		}
		if( term<from ) return String();
		if( from==0 && term==len ) return *this;
		return String( rep->data+from,term-from );
	}

	String Slice( int from )const{
		return Slice( from,rep->length );
	}
	
	Array<int> ToChars()const{
		Array<int> chars( rep->length );
		for( int i=0;i<rep->length;++i ) chars[i]=rep->data[i];
		return chars;
	}
	
	int ToInt()const{
		char buf[64];
		return atoi( ToCString<char>( buf,sizeof(buf) ) );
	}
	
	Float ToFloat()const{
		char buf[256];
		return atof( ToCString<char>( buf,sizeof(buf) ) );
	}

	template<class C> class CString{
		struct Rep{
			int refs;
			C data[1];
		};
		Rep *_rep;
		static Rep _nul;
	public:
		template<class T> CString( const T *data,int length ){
			_rep=(Rep*)malloc( length*sizeof(C)+sizeof(Rep) );
			_rep->refs=1;
			_rep->data[length]=0;
			for( int i=0;i<length;++i ){
				_rep->data[i]=(C)data[i];
			}
		}
		CString():_rep( new Rep ){
			_rep->refs=1;
		}
		CString( const CString &c ):_rep(c._rep){
			++_rep->refs;
		}
		~CString(){
			if( !--_rep->refs ) free( _rep );
		}
		CString &operator=( const CString &c ){
			++c._rep->refs;
			if( !--_rep->refs ) free( _rep );
			_rep=c._rep;
			return *this;
		}
		operator const C*()const{ 
			return _rep->data;
		}
	};
	
	template<class C> CString<C> ToCString()const{
		return CString<C>( rep->data,rep->length );
	}

	template<class C> C *ToCString( C *p,int length )const{
		if( --length>rep->length ) length=rep->length;
		for( int i=0;i<length;++i ) p[i]=rep->data[i];
		p[length]=0;
		return p;
	}
	
#if __OBJC__	
	NSString *ToNSString()const{
		return [NSString stringWithCharacters:ToCString<unichar>() length:rep->length];
	}
#endif

#if __cplusplus_winrt
	Platform::String ^ToWinRTString()const{
		return ref new Platform::String( rep->data,rep->length );
	}
#endif
	CString<char> ToUtf8()const{
		std::vector<unsigned char> buf;
		Save( buf );
		return CString<char>( &buf[0],buf.size() );
	}

	bool Save( FILE *fp )const{
		std::vector<unsigned char> buf;
		Save( buf );
		return buf.size() ? fwrite( &buf[0],1,buf.size(),fp )==buf.size() : true;
	}
	
	void Save( std::vector<unsigned char> &buf )const{
	
		Char *p=rep->data;
		Char *e=p+rep->length;
		
		while( p<e ){
			Char c=*p++;
			if( c<0x80 ){
				buf.push_back( c );
			}else if( c<0x800 ){
				buf.push_back( 0xc0 | (c>>6) );
				buf.push_back( 0x80 | (c & 0x3f) );
			}else{
				buf.push_back( 0xe0 | (c>>12) );
				buf.push_back( 0x80 | ((c>>6) & 0x3f) );
				buf.push_back( 0x80 | (c & 0x3f) );
			}
		}
	}
	
	static String FromChars( Array<int> chars ){
		int n=chars.Length();
		Rep *p=Rep::alloc( n );
		for( int i=0;i<n;++i ){
			p->data[i]=chars[i];
		}
		return String( p );
	}

	static String Load( FILE *fp ){
		unsigned char tmp[4096];
		std::vector<unsigned char> buf;
		for(;;){
			int n=fread( tmp,1,4096,fp );
			if( n>0 ) buf.insert( buf.end(),tmp,tmp+n );
			if( n!=4096 ) break;
		}
		return buf.size() ? String::Load( &buf[0],buf.size() ) : String();
	}
	
	static String Load( unsigned char *p,int n ){
	
		_str_load_err=0;
		
		unsigned char *e=p+n;
		std::vector<Char> chars;
		
		int t0=n>0 ? p[0] : -1;
		int t1=n>1 ? p[1] : -1;

		if( t0==0xfe && t1==0xff ){
			p+=2;
			while( p<e-1 ){
				int c=*p++;
				chars.push_back( (c<<8)|*p++ );
			}
		}else if( t0==0xff && t1==0xfe ){
			p+=2;
			while( p<e-1 ){
				int c=*p++;
				chars.push_back( (*p++<<8)|c );
			}
		}else{
			int t2=n>2 ? p[2] : -1;
			if( t0==0xef && t1==0xbb && t2==0xbf ) p+=3;
			unsigned char *q=p;
			bool fail=false;
			while( p<e ){
				unsigned int c=*p++;
				if( c & 0x80 ){
					if( (c & 0xe0)==0xc0 ){
						if( p>=e || (p[0] & 0xc0)!=0x80 ){
							fail=true;
							break;
						}
						c=((c & 0x1f)<<6) | (p[0] & 0x3f);
						p+=1;
					}else if( (c & 0xf0)==0xe0 ){
						if( p+1>=e || (p[0] & 0xc0)!=0x80 || (p[1] & 0xc0)!=0x80 ){
							fail=true;
							break;
						}
						c=((c & 0x0f)<<12) | ((p[0] & 0x3f)<<6) | (p[1] & 0x3f);
						p+=2;
					}else{
						fail=true;
						break;
					}
				}
				chars.push_back( c );
			}
			if( fail ){
				_str_load_err="Invalid UTF-8";
				return String( q,n );
			}
		}
		return chars.size() ? String( &chars[0],chars.size() ) : String();
	}

private:
	
	struct Rep{
		int refs;
		int length;
		Char data[0];
		
		Rep():refs(1),length(0){
		}
		
		Rep( int length ):refs(1),length(length){
		}
		
		void retain(){
//			atomic_add( &refs,1 );
			++refs;
		}
		
		void release(){
//			if( atomic_sub( &refs,1 )>1 || this==&nullRep ) return;
			if( --refs || this==&nullRep ) return;
			free( this );
		}

		static Rep *alloc( int length ){
			if( !length ) return &nullRep;
			void *p=malloc( sizeof(Rep)+length*sizeof(Char) );
			return new(p) Rep( length );
		}
	};
	Rep *rep;
	
	static Rep nullRep;
	
	String( Rep *rep ):rep(rep){
	}
};

String::Rep String::nullRep;

String *t_create( int n,String *p ){
	for( int i=0;i<n;++i ) new( &p[i] ) String();
	return p+n;
}

String *t_create( int n,String *p,const String *q ){
	for( int i=0;i<n;++i ) new( &p[i] ) String( q[i] );
	return p+n;
}

void t_destroy( int n,String *p ){
	for( int i=0;i<n;++i ) p[i].~String();
}

// ***** Object *****

String dbg_stacktrace();

class Object : public gc_object{
public:
	virtual bool Equals( Object *obj ){
		return this==obj;
	}
	
	virtual int Compare( Object *obj ){
		return (char*)this-(char*)obj;
	}
	
	virtual String debug(){
		return "+Object\n";
	}
};

class ThrowableObject : public Object{
#ifndef NDEBUG
public:
	String stackTrace;
	ThrowableObject():stackTrace( dbg_stacktrace() ){}
#endif
};

struct gc_interface{
	virtual ~gc_interface(){}
};

//***** Debugger *****

//#define Error bbError
//#define Print bbPrint

int bbPrint( String t );

#define dbg_stream stderr

#if _MSC_VER
#define dbg_typeof decltype
#else
#define dbg_typeof __typeof__
#endif 

struct dbg_func;
struct dbg_var_type;

static int dbg_suspend;
static int dbg_stepmode;

const char *dbg_info;
String dbg_exstack;

static void *dbg_var_buf[65536*3];
static void **dbg_var_ptr=dbg_var_buf;

static dbg_func *dbg_func_buf[1024];
static dbg_func **dbg_func_ptr=dbg_func_buf;

String dbg_type( bool *p ){
	return "Bool";
}

String dbg_type( int *p ){
	return "Int";
}

String dbg_type( Float *p ){
	return "Float";
}

String dbg_type( String *p ){
	return "String";
}

template<class T> String dbg_type( T **p ){
	return "Object";
}

template<class T> String dbg_type( Array<T> *p ){
	return dbg_type( &(*p)[0] )+"[]";
}

String dbg_value( bool *p ){
	return *p ? "True" : "False";
}

String dbg_value( int *p ){
	return String( *p );
}

String dbg_value( Float *p ){
	return String( *p );
}

String dbg_value( String *p ){
	String t=*p;
	if( t.Length()>100 ) t=t.Slice( 0,100 )+"...";
	t=t.Replace( "\"","~q" );
	t=t.Replace( "\t","~t" );
	t=t.Replace( "\n","~n" );
	t=t.Replace( "\r","~r" );
	return String("\"")+t+"\"";
}

template<class T> String dbg_value( T **t ){
	Object *p=dynamic_cast<Object*>( *t );
	char buf[64];
	sprintf_s( buf,"%p",p );
	return String("@") + (buf[0]=='0' && buf[1]=='x' ? buf+2 : buf );
}

template<class T> String dbg_value( Array<T> *p ){
	String t="[";
	int n=(*p).Length();
	if( n>100 ) n=100;
	for( int i=0;i<n;++i ){
		if( i ) t+=",";
		t+=dbg_value( &(*p)[i] );
	}
	return t+"]";
}

String dbg_ptr_value( void *p ){
	char buf[64];
	sprintf_s( buf,"%p",p );
	return (buf[0]=='0' && buf[1]=='x' ? buf+2 : buf );
}

template<class T> String dbg_decl( const char *id,T *ptr ){
	return String( id )+":"+dbg_type(ptr)+"="+dbg_value(ptr)+"\n";
}

struct dbg_var_type{
	virtual String type( void *p )=0;
	virtual String value( void *p )=0;
};

template<class T> struct dbg_var_type_t : public dbg_var_type{

	String type( void *p ){
		return dbg_type( (T*)p );
	}
	
	String value( void *p ){
		return dbg_value( (T*)p );
	}
	
	static dbg_var_type_t<T> info;
};
template<class T> dbg_var_type_t<T> dbg_var_type_t<T>::info;

struct dbg_blk{
	void **var_ptr;
	
	dbg_blk():var_ptr(dbg_var_ptr){
		if( dbg_stepmode=='l' ) --dbg_suspend;
	}
	
	~dbg_blk(){
		if( dbg_stepmode=='l' ) ++dbg_suspend;
		dbg_var_ptr=var_ptr;
	}
};

struct dbg_func : public dbg_blk{
	const char *id;
	const char *info;

	dbg_func( const char *p ):id(p),info(dbg_info){
		*dbg_func_ptr++=this;
		if( dbg_stepmode=='s' ) --dbg_suspend;
	}
	
	~dbg_func(){
		if( dbg_stepmode=='s' ) ++dbg_suspend;
		--dbg_func_ptr;
		dbg_info=info;
	}
};

int dbg_print( String t ){
	static char *buf;
	static int len;
	int n=t.Length();
	if( n+100>len ){
		len=n+100;
		free( buf );
		buf=(char*)malloc( len );
	}
	buf[n]='\n';
	for( int i=0;i<n;++i ) buf[i]=t[i];
	fwrite( buf,n+1,1,dbg_stream );
	fflush( dbg_stream );
	return 0;
}

void dbg_callstack(){

	void **var_ptr=dbg_var_buf;
	dbg_func **func_ptr=dbg_func_buf;
	
	while( var_ptr!=dbg_var_ptr ){
		while( func_ptr!=dbg_func_ptr && var_ptr==(*func_ptr)->var_ptr ){
			const char *id=(*func_ptr++)->id;
			const char *info=func_ptr!=dbg_func_ptr ? (*func_ptr)->info : dbg_info;
			fprintf( dbg_stream,"+%s;%s\n",id,info );
		}
		void *vp=*var_ptr++;
		const char *nm=(const char*)*var_ptr++;
		dbg_var_type *ty=(dbg_var_type*)*var_ptr++;
		dbg_print( String(nm)+":"+ty->type(vp)+"="+ty->value(vp) );
	}
	while( func_ptr!=dbg_func_ptr ){
		const char *id=(*func_ptr++)->id;
		const char *info=func_ptr!=dbg_func_ptr ? (*func_ptr)->info : dbg_info;
		fprintf( dbg_stream,"+%s;%s\n",id,info );
	}
}

String dbg_stacktrace(){
	if( !dbg_info || !dbg_info[0] ) return "";
	String str=String( dbg_info )+"\n";
	dbg_func **func_ptr=dbg_func_ptr;
	if( func_ptr==dbg_func_buf ) return str;
	while( --func_ptr!=dbg_func_buf ){
		str+=String( (*func_ptr)->info )+"\n";
	}
	return str;
}

void dbg_throw( const char *err ){
	dbg_exstack=dbg_stacktrace();
	throw err;
}

void dbg_stop(){

#if TARGET_OS_IPHONE
	dbg_throw( "STOP" );
#endif

	fprintf( dbg_stream,"{{~~%s~~}}\n",dbg_info );
	dbg_callstack();
	dbg_print( "" );
	
	for(;;){

		char buf[256];
		char *e=fgets( buf,256,stdin );
		if( !e ) exit( -1 );
		
		e=strchr( buf,'\n' );
		if( !e ) exit( -1 );
		
		*e=0;
		
		Object *p;
		
		switch( buf[0] ){
		case '?':
			break;
		case 'r':	//run
			dbg_suspend=0;		
			dbg_stepmode=0;
			return;
		case 's':	//step
			dbg_suspend=1;
			dbg_stepmode='s';
			return;
		case 'e':	//enter func
			dbg_suspend=1;
			dbg_stepmode='e';
			return;
		case 'l':	//leave block
			dbg_suspend=0;
			dbg_stepmode='l';
			return;
		case '@':	//dump object
			p=0;
			sscanf_s( buf+1,"%p",&p );
			if( p ){
				dbg_print( p->debug() );
			}else{
				dbg_print( "" );
			}
			break;
		case 'q':	//quit!
			exit( 0 );
			break;			
		default:
			printf( "????? %s ?????",buf );fflush( stdout );
			exit( -1 );
		}
	}
}

void dbg_error( const char *err ){

#if TARGET_OS_IPHONE
	dbg_throw( err );
#endif

	for(;;){
		bbPrint( String("Monkey Runtime Error : ")+err );
		bbPrint( dbg_stacktrace() );
		dbg_stop();
	}
}

#define DBG_INFO(X) dbg_info=(X);if( dbg_suspend>0 ) dbg_stop();

#define DBG_ENTER(P) dbg_func _dbg_func(P);

#define DBG_BLOCK() dbg_blk _dbg_blk;

#define DBG_GLOBAL( ID,NAME )	//TODO!

#define DBG_LOCAL( ID,NAME )\
*dbg_var_ptr++=&ID;\
*dbg_var_ptr++=(void*)NAME;\
*dbg_var_ptr++=&dbg_var_type_t<dbg_typeof(ID)>::info;

//**** main ****

int argc;
const char **argv;

Float D2R=0.017453292519943295f;
Float R2D=57.29577951308232f;

int bbPrint( String t ){

	static std::vector<unsigned char> buf;
	buf.clear();
	t.Save( buf );
	buf.push_back( '\n' );
	buf.push_back( 0 );
	
#if __cplusplus_winrt	//winrt?

#if CFG_WINRT_PRINT_ENABLED
	OutputDebugStringA( (const char*)&buf[0] );
#endif

#elif _WIN32			//windows?

	fputs( (const char*)&buf[0],stdout );
	fflush( stdout );

#elif __APPLE__			//macos/ios?

	fputs( (const char*)&buf[0],stdout );
	fflush( stdout );
	
#elif __linux			//linux?

#if CFG_ANDROID_NDK_PRINT_ENABLED
	LOGI( (const char*)&buf[0] );
#else
	fputs( (const char*)&buf[0],stdout );
	fflush( stdout );
#endif

#endif

	return 0;
}

class BBExitApp{
};

int bbError( String err ){
	if( !err.Length() ){
#if __cplusplus_winrt
		throw BBExitApp();
#else
		exit( 0 );
#endif
	}
	dbg_error( err.ToCString<char>() );
	return 0;
}

int bbDebugLog( String t ){
	bbPrint( t );
	return 0;
}

int bbDebugStop(){
	dbg_stop();
	return 0;
}

int bbInit();
int bbMain();

#if _MSC_VER

static void _cdecl seTranslator( unsigned int ex,EXCEPTION_POINTERS *p ){

	switch( ex ){
	case EXCEPTION_ACCESS_VIOLATION:dbg_error( "Memory access violation" );
	case EXCEPTION_ILLEGAL_INSTRUCTION:dbg_error( "Illegal instruction" );
	case EXCEPTION_INT_DIVIDE_BY_ZERO:dbg_error( "Integer divide by zero" );
	case EXCEPTION_STACK_OVERFLOW:dbg_error( "Stack overflow" );
	}
	dbg_error( "Unknown exception" );
}

#else

void sighandler( int sig  ){
	switch( sig ){
	case SIGSEGV:dbg_error( "Memory access violation" );
	case SIGILL:dbg_error( "Illegal instruction" );
	case SIGFPE:dbg_error( "Floating point exception" );
#if !_WIN32
	case SIGBUS:dbg_error( "Bus error" );
#endif	
	}
	dbg_error( "Unknown signal" );
}

#endif

//entry point call by target main()...
//
int bb_std_main( int argc,const char **argv ){

	::argc=argc;
	::argv=argv;
	
#if _MSC_VER

	_set_se_translator( seTranslator );

#else
	
	signal( SIGSEGV,sighandler );
	signal( SIGILL,sighandler );
	signal( SIGFPE,sighandler );
#if !_WIN32
	signal( SIGBUS,sighandler );
#endif

#endif

	if( !setlocale( LC_CTYPE,"en_US.UTF-8" ) ){
		setlocale( LC_CTYPE,"" );
	}

	gc_init1();

	bbInit();
	
	gc_init2();

	bbMain();

	return 0;
}


//***** game.h *****

struct BBGameEvent{
	enum{
		None=0,
		KeyDown=1,KeyUp=2,KeyChar=3,
		MouseDown=4,MouseUp=5,MouseMove=6,
		TouchDown=7,TouchUp=8,TouchMove=9,
		MotionAccel=10
	};
};

class BBGameDelegate : public Object{
public:
	virtual void StartGame(){}
	virtual void SuspendGame(){}
	virtual void ResumeGame(){}
	virtual void UpdateGame(){}
	virtual void RenderGame(){}
	virtual void KeyEvent( int event,int data ){}
	virtual void MouseEvent( int event,int data,Float x,Float y ){}
	virtual void TouchEvent( int event,int data,Float x,Float y ){}
	virtual void MotionEvent( int event,int data,Float x,Float y,Float z ){}
	virtual void DiscardGraphics(){}
};

struct BBDisplayMode : public Object{
	int width;
	int height;
	int depth;
	int hertz;
	int flags;
	BBDisplayMode( int width=0,int height=0,int depth=0,int hertz=0,int flags=0 ):width(width),height(height),depth(depth),hertz(hertz),flags(flags){}
};

class BBGame{
public:
	BBGame();
	virtual ~BBGame(){}
	
	// ***** Extern *****
	static BBGame *Game(){ return _game; }
	
	virtual void SetDelegate( BBGameDelegate *delegate );
	virtual BBGameDelegate *Delegate(){ return _delegate; }
	
	virtual void SetKeyboardEnabled( bool enabled );
	virtual bool KeyboardEnabled();
	
	virtual void SetUpdateRate( int updateRate );
	virtual int UpdateRate();
	
	virtual bool Started(){ return _started; }
	virtual bool Suspended(){ return _suspended; }
	
	virtual int Millisecs();
	virtual void GetDate( Array<int> date );
	virtual int SaveState( String state );
	virtual String LoadState();
	virtual String LoadString( String path );
	virtual bool PollJoystick( int port,Array<Float> joyx,Array<Float> joyy,Array<Float> joyz,Array<bool> buttons );
	virtual void OpenUrl( String url );
	virtual void SetMouseVisible( bool visible );
	
	virtual int GetDeviceWidth(){ return 0; }
	virtual int GetDeviceHeight(){ return 0; }
	virtual void SetDeviceWindow( int width,int height,int flags ){}
	virtual Array<BBDisplayMode*> GetDisplayModes(){ return Array<BBDisplayMode*>(); }
	virtual BBDisplayMode *GetDesktopMode(){ return 0; }
	virtual void SetSwapInterval( int interval ){}

	// ***** Native *****
	virtual String PathToFilePath( String path );
	virtual FILE *OpenFile( String path,String mode );
	virtual unsigned char *LoadData( String path,int *plength );
	virtual unsigned char *LoadImageData( String path,int *width,int *height,int *depth ){ return 0; }
	virtual unsigned char *LoadAudioData( String path,int *length,int *channels,int *format,int *hertz ){ return 0; }
	
	//***** Internal *****
	virtual void Die( ThrowableObject *ex );
	virtual void gc_collect();
	virtual void StartGame();
	virtual void SuspendGame();
	virtual void ResumeGame();
	virtual void UpdateGame();
	virtual void RenderGame();
	virtual void KeyEvent( int ev,int data );
	virtual void MouseEvent( int ev,int data,float x,float y );
	virtual void TouchEvent( int ev,int data,float x,float y );
	virtual void MotionEvent( int ev,int data,float x,float y,float z );
	virtual void DiscardGraphics();
	
protected:

	static BBGame *_game;

	BBGameDelegate *_delegate;
	bool _keyboardEnabled;
	int _updateRate;
	bool _started;
	bool _suspended;
};

//***** game.cpp *****

BBGame *BBGame::_game;

BBGame::BBGame():
_delegate( 0 ),
_keyboardEnabled( false ),
_updateRate( 0 ),
_started( false ),
_suspended( false ){
	_game=this;
}

void BBGame::SetDelegate( BBGameDelegate *delegate ){
	_delegate=delegate;
}

void BBGame::SetKeyboardEnabled( bool enabled ){
	_keyboardEnabled=enabled;
}

bool BBGame::KeyboardEnabled(){
	return _keyboardEnabled;
}

void BBGame::SetUpdateRate( int updateRate ){
	_updateRate=updateRate;
}

int BBGame::UpdateRate(){
	return _updateRate;
}

int BBGame::Millisecs(){
	return 0;
}

void BBGame::GetDate( Array<int> date ){
	int n=date.Length();
	if( n>0 ){
		time_t t=time( 0 );
		
#if _MSC_VER
		struct tm tii;
		struct tm *ti=&tii;
		localtime_s( ti,&t );
#else
		struct tm *ti=localtime( &t );
#endif

		date[0]=ti->tm_year+1900;
		if( n>1 ){ 
			date[1]=ti->tm_mon+1;
			if( n>2 ){
				date[2]=ti->tm_mday;
				if( n>3 ){
					date[3]=ti->tm_hour;
					if( n>4 ){
						date[4]=ti->tm_min;
						if( n>5 ){
							date[5]=ti->tm_sec;
							if( n>6 ){
								date[6]=0;
							}
						}
					}
				}
			}
		}
	}
}

int BBGame::SaveState( String state ){
	if( FILE *f=OpenFile( "./.monkeystate","wb" ) ){
		bool ok=state.Save( f );
		fclose( f );
		return ok ? 0 : -2;
	}
	return -1;
}

String BBGame::LoadState(){
	if( FILE *f=OpenFile( "./.monkeystate","rb" ) ){
		String str=String::Load( f );
		fclose( f );
		return str;
	}
	return "";
}

String BBGame::LoadString( String path ){
	if( FILE *fp=OpenFile( path,"rb" ) ){
		String str=String::Load( fp );
		fclose( fp );
		return str;
	}
	return "";
}

bool BBGame::PollJoystick( int port,Array<Float> joyx,Array<Float> joyy,Array<Float> joyz,Array<bool> buttons ){
	return false;
}

void BBGame::OpenUrl( String url ){
}

void BBGame::SetMouseVisible( bool visible ){
}

//***** C++ Game *****

String BBGame::PathToFilePath( String path ){
	return path;
}

FILE *BBGame::OpenFile( String path,String mode ){
	path=PathToFilePath( path );
	if( path=="" ) return 0;
	
#if __cplusplus_winrt
	path=path.Replace( "/","\\" );
	FILE *f;
	if( _wfopen_s( &f,path.ToCString<wchar_t>(),mode.ToCString<wchar_t>() ) ) return 0;
	return f;
#elif _WIN32
	return _wfopen( path.ToCString<wchar_t>(),mode.ToCString<wchar_t>() );
#else
	return fopen( path.ToCString<char>(),mode.ToCString<char>() );
#endif
}

unsigned char *BBGame::LoadData( String path,int *plength ){

	FILE *f=OpenFile( path,"rb" );
	if( !f ) return 0;

	const int BUF_SZ=4096;
	std::vector<void*> tmps;
	int length=0;
	
	for(;;){
		void *p=malloc( BUF_SZ );
		int n=fread( p,1,BUF_SZ,f );
		tmps.push_back( p );
		length+=n;
		if( n!=BUF_SZ ) break;
	}
	fclose( f );
	
	unsigned char *data=(unsigned char*)malloc( length );
	unsigned char *p=data;
	
	int sz=length;
	for( int i=0;i<tmps.size();++i ){
		int n=sz>BUF_SZ ? BUF_SZ : sz;
		memcpy( p,tmps[i],n );
		free( tmps[i] );
		sz-=n;
		p+=n;
	}
	
	*plength=length;
	
	gc_ext_malloced( length );
	
	return data;
}

//***** INTERNAL *****

void BBGame::Die( ThrowableObject *ex ){
	bbPrint( "Monkey Runtime Error : Uncaught Monkey Exception" );
#ifndef NDEBUG
	bbPrint( ex->stackTrace );
#endif
	exit( -1 );
}

void BBGame::gc_collect(){
	gc_mark( _delegate );
	::gc_collect();
}

void BBGame::StartGame(){

	if( _started ) return;
	_started=true;
	
	try{
		_delegate->StartGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::SuspendGame(){

	if( !_started || _suspended ) return;
	_suspended=true;
	
	try{
		_delegate->SuspendGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::ResumeGame(){

	if( !_started || !_suspended ) return;
	_suspended=false;
	
	try{
		_delegate->ResumeGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::UpdateGame(){

	if( !_started || _suspended ) return;
	
	try{
		_delegate->UpdateGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::RenderGame(){

	if( !_started ) return;
	
	try{
		_delegate->RenderGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::KeyEvent( int ev,int data ){

	if( !_started ) return;
	
	try{
		_delegate->KeyEvent( ev,data );
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::MouseEvent( int ev,int data,float x,float y ){

	if( !_started ) return;
	
	try{
		_delegate->MouseEvent( ev,data,x,y );
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::TouchEvent( int ev,int data,float x,float y ){

	if( !_started ) return;
	
	try{
		_delegate->TouchEvent( ev,data,x,y );
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::MotionEvent( int ev,int data,float x,float y,float z ){

	if( !_started ) return;
	
	try{
		_delegate->MotionEvent( ev,data,x,y,z );
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::DiscardGraphics(){

	if( !_started ) return;
	
	try{
		_delegate->DiscardGraphics();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}


//***** wavloader.h *****
//
unsigned char *LoadWAV( FILE *f,int *length,int *channels,int *format,int *hertz );

//***** wavloader.cpp *****
//
static const char *readTag( FILE *f ){
	static char buf[8];
	if( fread( buf,4,1,f )!=1 ) return "";
	buf[4]=0;
	return buf;
}

static int readInt( FILE *f ){
	unsigned char buf[4];
	if( fread( buf,4,1,f )!=1 ) return -1;
	return (buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0];
}

static int readShort( FILE *f ){
	unsigned char buf[2];
	if( fread( buf,2,1,f )!=1 ) return -1;
	return (buf[1]<<8) | buf[0];
}

static void skipBytes( int n,FILE *f ){
	char *p=(char*)malloc( n );
	fread( p,n,1,f );
	free(p);
}

unsigned char *LoadWAV( FILE *f,int *plength,int *pchannels,int *pformat,int *phertz ){
	if( !strcmp( readTag( f ),"RIFF" ) ){
		int len=readInt( f )-8;len=len;
		if( !strcmp( readTag( f ),"WAVE" ) ){
			if( !strcmp( readTag( f ),"fmt " ) ){
				int len2=readInt( f );
				int comp=readShort( f );
				if( comp==1 ){
					int chans=readShort( f );
					int hertz=readInt( f );
					int bytespersec=readInt( f );bytespersec=bytespersec;
					int pad=readShort( f );pad=pad;
					int bits=readShort( f );
					int format=bits/8;
					if( len2>16 ) skipBytes( len2-16,f );
					for(;;){
						const char *p=readTag( f );
						if( feof( f ) ) break;
						int size=readInt( f );
						if( strcmp( p,"data" ) ){
							skipBytes( size,f );
							continue;
						}
						unsigned char *data=(unsigned char*)malloc( size );
						if( fread( data,size,1,f )==1 ){
							*plength=size/(chans*format);
							*pchannels=chans;
							*pformat=format;
							*phertz=hertz;
							return data;
						}
						free( data );
					}
				}
			}
		}
	}
	return 0;
}



//***** oggloader.h *****
unsigned char *LoadOGG( FILE *f,int *length,int *channels,int *format,int *hertz );

//***** oggloader.cpp *****
unsigned char *LoadOGG( FILE *f,int *length,int *channels,int *format,int *hertz ){

	int error;
	stb_vorbis *v=stb_vorbis_open_file( f,0,&error,0 );
	if( !v ) return 0;
	
	stb_vorbis_info info=stb_vorbis_get_info( v );
	
	int limit=info.channels*4096;
	int offset=0,data_len=0,total=limit;

	short *data=(short*)malloc( total*sizeof(short) );
	
	for(;;){
		int n=stb_vorbis_get_frame_short_interleaved( v,info.channels,data+offset,total-offset );
		if( !n ) break;
	
		data_len+=n;
		offset+=n*info.channels;
		
		if( offset+limit>total ){
			total*=2;
			data=(short*)realloc( data,total*sizeof(short) );
		}
	}
	
	*length=data_len;
	*channels=info.channels;
	*format=2;
	*hertz=info.sample_rate;
	
	stb_vorbis_close(v);

	return (unsigned char*)data;
}



//***** glfwgame.h *****

class BBGlfwGame : public BBGame{
public:
	BBGlfwGame();
	
	static BBGlfwGame *GlfwGame(){ return _glfwGame; }
	
	virtual void SetUpdateRate( int hertz );
	virtual int Millisecs();
	virtual bool PollJoystick( int port,Array<Float> joyx,Array<Float> joyy,Array<Float> joyz,Array<bool> buttons );
	virtual void OpenUrl( String url );
	virtual void SetMouseVisible( bool visible );
		
	virtual int GetDeviceWidth(){ return _width; }
	virtual int GetDeviceHeight(){ return _height; }
	virtual void SetDeviceWindow( int width,int height,int flags );
	virtual void SetSwapInterval( int interval );
	virtual Array<BBDisplayMode*> GetDisplayModes();
	virtual BBDisplayMode *GetDesktopMode();

	virtual String PathToFilePath( String path );
	virtual unsigned char *LoadImageData( String path,int *width,int *height,int *format );
	virtual unsigned char *LoadAudioData( String path,int *length,int *channels,int *format,int *hertz );
	
	void Run();
	
	GLFWwindow *GetGLFWwindow(){ return _window; }
		
private:
	static BBGlfwGame *_glfwGame;
	
	GLFWvidmode _desktopMode;
	
	GLFWwindow *_window;
	int _width;
	int _height;
	int _swapInterval;
	bool _focus;

	double _updatePeriod;
	double _nextUpdate;
	
	double GetTime();
	void Sleep( double time );
	void UpdateEvents();

//	void SetGlfwWindow( int width,int height,int red,int green,int blue,int alpha,int depth,int stencil,bool fullscreen );
		
	static int TransKey( int key );
	static int KeyToChar( int key );
	
	static void OnKey( GLFWwindow *window,int key,int scancode,int action,int mods );
	static void OnChar( GLFWwindow *window,unsigned int chr );
	static void OnMouseButton( GLFWwindow *window,int button,int action,int mods );
	static void OnCursorPos( GLFWwindow *window,double x,double y );
	static void OnWindowClose( GLFWwindow *window );
	static void OnWindowSize( GLFWwindow *window,int width,int height );
};

//***** glfwgame.cpp *****

#include <errno.h>

#define _QUOTE(X) #X
#define _STRINGIZE( X ) _QUOTE(X)

enum{
	VKEY_BACKSPACE=8,VKEY_TAB,
	VKEY_ENTER=13,
	VKEY_SHIFT=16,
	VKEY_CONTROL=17,
	VKEY_ESCAPE=27,
	VKEY_SPACE=32,
	VKEY_PAGE_UP=33,VKEY_PAGE_DOWN,VKEY_END,VKEY_HOME,
	VKEY_LEFT=37,VKEY_UP,VKEY_RIGHT,VKEY_DOWN,
	VKEY_INSERT=45,VKEY_DELETE,
	VKEY_0=48,VKEY_1,VKEY_2,VKEY_3,VKEY_4,VKEY_5,VKEY_6,VKEY_7,VKEY_8,VKEY_9,
	VKEY_A=65,VKEY_B,VKEY_C,VKEY_D,VKEY_E,VKEY_F,VKEY_G,VKEY_H,VKEY_I,VKEY_J,
	VKEY_K,VKEY_L,VKEY_M,VKEY_N,VKEY_O,VKEY_P,VKEY_Q,VKEY_R,VKEY_S,VKEY_T,
	VKEY_U,VKEY_V,VKEY_W,VKEY_X,VKEY_Y,VKEY_Z,
	
	VKEY_LSYS=91,VKEY_RSYS,
	
	VKEY_NUM0=96,VKEY_NUM1,VKEY_NUM2,VKEY_NUM3,VKEY_NUM4,
	VKEY_NUM5,VKEY_NUM6,VKEY_NUM7,VKEY_NUM8,VKEY_NUM9,
	VKEY_NUMMULTIPLY=106,VKEY_NUMADD,VKEY_NUMSLASH,
	VKEY_NUMSUBTRACT,VKEY_NUMDECIMAL,VKEY_NUMDIVIDE,

	VKEY_F1=112,VKEY_F2,VKEY_F3,VKEY_F4,VKEY_F5,VKEY_F6,
	VKEY_F7,VKEY_F8,VKEY_F9,VKEY_F10,VKEY_F11,VKEY_F12,

	VKEY_LEFT_SHIFT=160,VKEY_RIGHT_SHIFT,
	VKEY_LEFT_CONTROL=162,VKEY_RIGHT_CONTROL,
	VKEY_LEFT_ALT=164,VKEY_RIGHT_ALT,

	VKEY_TILDE=192,VKEY_MINUS=189,VKEY_EQUALS=187,
	VKEY_OPENBRACKET=219,VKEY_BACKSLASH=220,VKEY_CLOSEBRACKET=221,
	VKEY_SEMICOLON=186,VKEY_QUOTES=222,
	VKEY_COMMA=188,VKEY_PERIOD=190,VKEY_SLASH=191
};

void Init_GL_Exts();

int glfwGraphicsSeq=0;

BBGlfwGame *BBGlfwGame::_glfwGame;

BBGlfwGame::BBGlfwGame():_window(0),_width(0),_height(0),_swapInterval(1),_focus(true),_updatePeriod(0),_nextUpdate(0){
	_glfwGame=this;

	memset( &_desktopMode,0,sizeof(_desktopMode) );	
	const GLFWvidmode *vmode=glfwGetVideoMode( glfwGetPrimaryMonitor() );
	if( vmode ) _desktopMode=*vmode;
}

void BBGlfwGame::SetUpdateRate( int updateRate ){
	BBGame::SetUpdateRate( updateRate );
	if( _updateRate ) _updatePeriod=1.0/_updateRate;
	_nextUpdate=0;
}

int BBGlfwGame::Millisecs(){
	return int( GetTime()*1000.0 );
}

bool BBGlfwGame::PollJoystick( int port,Array<Float> joyx,Array<Float> joyy,Array<Float> joyz,Array<bool> buttons ){

	static int pjoys[4];
	if( !port ){
		int i=0;
		for( int joy=0;joy<16 && i<4;++joy ){
			if( glfwJoystickPresent( joy ) ) pjoys[i++]=joy;
		}
		while( i<4 ) pjoys[i++]=-1;
	}
	port=pjoys[port];
	if( port==-1 ) return false;
	
	//read axes
	int n_axes=0;
	const float *axes=glfwGetJoystickAxes( port,&n_axes );
	
	//read buttons
	int n_buts=0;
	const unsigned char *buts=glfwGetJoystickButtons( port,&n_buts );

	//Ugh...
	
	const int *dev_axes;
	const int *dev_buttons;
	
#if _WIN32
	
	//xbox 360 controller
	const int xbox360_axes[]={0,0x41,2,4,0x43,0x42,999};
	const int xbox360_buttons[]={0,1,2,3,4,5,6,7,13,10,11,12,8,9,999};
	
	//logitech dual action
	const int logitech_axes[]={0,1,0x86,2,0x43,0x87,999};
	const int logitech_buttons[]={1,2,0,3,4,5,8,9,15,12,13,14,10,11,999};
	
	if( n_axes==5 && n_buts==14 ){
		dev_axes=xbox360_axes;
		dev_buttons=xbox360_buttons;
	}else{
		dev_axes=logitech_axes;
		dev_buttons=logitech_buttons;
	}
	
#else

	//xbox 360 controller
	const int xbox360_axes[]={0,0x41,0x14,2,0x43,0x15,999};
	const int xbox360_buttons[]={11,12,13,14,8,9,5,4,2,0,3,1,6,7,10,999};

	//ps3 controller
	const int ps3_axes[]={0,0x41,0x88,2,0x43,0x89,999};
	const int ps3_buttons[]={14,13,15,12,10,11,0,3,7,4,5,6,1,2,16,999};

	//logitech dual action
	const int logitech_axes[]={0,0x41,0x86,2,0x43,0x87,999};
	const int logitech_buttons[]={1,2,0,3,4,5,8,9,15,12,13,14,10,11,999};

	if( n_axes==6 && n_buts==15 ){
		dev_axes=xbox360_axes;
		dev_buttons=xbox360_buttons;
	}else if( n_axes==4 && n_buts==19 ){
		dev_axes=ps3_axes;
		dev_buttons=ps3_buttons;
	}else{
		dev_axes=logitech_axes;
		dev_buttons=logitech_buttons;
	}

#endif

	const int *p=dev_axes;
	
	float joys[6]={0,0,0,0,0,0};
	
	for( int i=0;i<6 && p[i]!=999;++i ){
		int j=p[i]&0xf,k=p[i]&~0xf;
		if( k==0x10 ){
			joys[i]=(axes[j]+1)/2;
		}else if( k==0x20 ){
			joys[i]=(1-axes[j])/2;
		}else if( k==0x40 ){
			joys[i]=-axes[j];
		}else if( k==0x80 ){
			joys[i]=(buts[j]==GLFW_PRESS);
		}else{
			joys[i]=axes[j];
		}
	}
	
	joyx[0]=joys[0];joyy[0]=joys[1];joyz[0]=joys[2];
	joyx[1]=joys[3];joyy[1]=joys[4];joyz[1]=joys[5];
	
	p=dev_buttons;
	
	for( int i=0;i<32;++i ) buttons[i]=false;
	
	for( int i=0;i<32 && p[i]!=999;++i ){
		int j=p[i];
		if( j<0 ) j+=n_buts;
		buttons[i]=(buts[j]==GLFW_PRESS);
	}

	return true;
}

void BBGlfwGame::OpenUrl( String url ){
#if _WIN32
	ShellExecute( HWND_DESKTOP,"open",url.ToCString<char>(),0,0,SW_SHOWNORMAL );
#elif __APPLE__
	if( CFURLRef cfurl=CFURLCreateWithBytes( 0,url.ToCString<UInt8>(),url.Length(),kCFStringEncodingASCII,0 ) ){
		LSOpenCFURLRef( cfurl,0 );
		CFRelease( cfurl );
	}
#elif __linux
	system( ( String( "xdg-open \"" )+url+"\"" ).ToCString<char>() );
#endif
}

void BBGlfwGame::SetMouseVisible( bool visible ){
	if( visible ){
		glfwSetInputMode( _window,GLFW_CURSOR,GLFW_CURSOR_NORMAL );
	}else{
		glfwSetInputMode( _window,GLFW_CURSOR,GLFW_CURSOR_HIDDEN );
	}
}

String BBGlfwGame::PathToFilePath( String path ){
	if( !path.StartsWith( "monkey:" ) ){
		return path;
	}else if( path.StartsWith( "monkey://data/" ) ){
		return String("./data/")+path.Slice( 14 );
	}else if( path.StartsWith( "monkey://internal/" ) ){
		return String("./internal/")+path.Slice( 18 );
	}else if( path.StartsWith( "monkey://external/" ) ){
		return String("./external/")+path.Slice( 18 );
	}
	return "";
}

unsigned char *BBGlfwGame::LoadImageData( String path,int *width,int *height,int *depth ){

	FILE *f=OpenFile( path,"rb" );
	if( !f ) return 0;
	
	unsigned char *data=stbi_load_from_file( f,width,height,depth,0 );
	fclose( f );
	
	if( data ) gc_ext_malloced( (*width)*(*height)*(*depth) );
	
	return data;
}

/*
extern "C" unsigned char *load_image_png( FILE *f,int *width,int *height,int *format );
extern "C" unsigned char *load_image_jpg( FILE *f,int *width,int *height,int *format );

unsigned char *BBGlfwGame::LoadImageData( String path,int *width,int *height,int *depth ){

	FILE *f=OpenFile( path,"rb" );
	if( !f ) return 0;

	unsigned char *data=0;
	
	if( path.ToLower().EndsWith( ".png" ) ){
		data=load_image_png( f,width,height,depth );
	}else if( path.ToLower().EndsWith( ".jpg" ) || path.ToLower().EndsWith( ".jpeg" ) ){
		data=load_image_jpg( f,width,height,depth );
	}else{
		//meh?
	}

	fclose( f );
	
	if( data ) gc_ext_malloced( (*width)*(*height)*(*depth) );
	
	return data;
}
*/

unsigned char *BBGlfwGame::LoadAudioData( String path,int *length,int *channels,int *format,int *hertz ){

	FILE *f=OpenFile( path,"rb" );
	if( !f ) return 0;
	
	unsigned char *data=0;
	
	if( path.ToLower().EndsWith( ".wav" ) ){
		data=LoadWAV( f,length,channels,format,hertz );
	}else if( path.ToLower().EndsWith( ".ogg" ) ){
		data=LoadOGG( f,length,channels,format,hertz );
	}
	fclose( f );
	
	if( data ) gc_ext_malloced( (*length)*(*channels)*(*format) );
	
	return data;
}

//glfw key to monkey key!
int BBGlfwGame::TransKey( int key ){

	if( key>='0' && key<='9' ) return key;
	if( key>='A' && key<='Z' ) return key;

	switch( key ){
	case ' ':return VKEY_SPACE;
	case ';':return VKEY_SEMICOLON;
	case '=':return VKEY_EQUALS;
	case ',':return VKEY_COMMA;
	case '-':return VKEY_MINUS;
	case '.':return VKEY_PERIOD;
	case '/':return VKEY_SLASH;
	case '~':return VKEY_TILDE;
	case '[':return VKEY_OPENBRACKET;
	case ']':return VKEY_CLOSEBRACKET;
	case '\"':return VKEY_QUOTES;
	case '\\':return VKEY_BACKSLASH;
	
	case '`':return VKEY_TILDE;
	case '\'':return VKEY_QUOTES;

	case GLFW_KEY_LEFT_SHIFT:
	case GLFW_KEY_RIGHT_SHIFT:return VKEY_SHIFT;
	case GLFW_KEY_LEFT_CONTROL:
	case GLFW_KEY_RIGHT_CONTROL:return VKEY_CONTROL;
	
//	case GLFW_KEY_LEFT_SHIFT:return VKEY_LEFT_SHIFT;
//	case GLFW_KEY_RIGHT_SHIFT:return VKEY_RIGHT_SHIFT;
//	case GLFW_KEY_LCTRL:return VKEY_LCONTROL;
//	case GLFW_KEY_RCTRL:return VKEY_RCONTROL;
	
	case GLFW_KEY_BACKSPACE:return VKEY_BACKSPACE;
	case GLFW_KEY_TAB:return VKEY_TAB;
	case GLFW_KEY_ENTER:return VKEY_ENTER;
	case GLFW_KEY_ESCAPE:return VKEY_ESCAPE;
	case GLFW_KEY_INSERT:return VKEY_INSERT;
	case GLFW_KEY_DELETE:return VKEY_DELETE;
	case GLFW_KEY_PAGE_UP:return VKEY_PAGE_UP;
	case GLFW_KEY_PAGE_DOWN:return VKEY_PAGE_DOWN;
	case GLFW_KEY_HOME:return VKEY_HOME;
	case GLFW_KEY_END:return VKEY_END;
	case GLFW_KEY_UP:return VKEY_UP;
	case GLFW_KEY_DOWN:return VKEY_DOWN;
	case GLFW_KEY_LEFT:return VKEY_LEFT;
	case GLFW_KEY_RIGHT:return VKEY_RIGHT;
	
	case GLFW_KEY_KP_0:return VKEY_NUM0;
	case GLFW_KEY_KP_1:return VKEY_NUM1;
	case GLFW_KEY_KP_2:return VKEY_NUM2;
	case GLFW_KEY_KP_3:return VKEY_NUM3;
	case GLFW_KEY_KP_4:return VKEY_NUM4;
	case GLFW_KEY_KP_5:return VKEY_NUM5;
	case GLFW_KEY_KP_6:return VKEY_NUM6;
	case GLFW_KEY_KP_7:return VKEY_NUM7;
	case GLFW_KEY_KP_8:return VKEY_NUM8;
	case GLFW_KEY_KP_9:return VKEY_NUM9;
	case GLFW_KEY_KP_DIVIDE:return VKEY_NUMDIVIDE;
	case GLFW_KEY_KP_MULTIPLY:return VKEY_NUMMULTIPLY;
	case GLFW_KEY_KP_SUBTRACT:return VKEY_NUMSUBTRACT;
	case GLFW_KEY_KP_ADD:return VKEY_NUMADD;
	case GLFW_KEY_KP_DECIMAL:return VKEY_NUMDECIMAL;
	
	case GLFW_KEY_F1:return VKEY_F1;
	case GLFW_KEY_F2:return VKEY_F2;
	case GLFW_KEY_F3:return VKEY_F3;
	case GLFW_KEY_F4:return VKEY_F4;
	case GLFW_KEY_F5:return VKEY_F5;
	case GLFW_KEY_F6:return VKEY_F6;
	case GLFW_KEY_F7:return VKEY_F7;
	case GLFW_KEY_F8:return VKEY_F8;
	case GLFW_KEY_F9:return VKEY_F9;
	case GLFW_KEY_F10:return VKEY_F10;
	case GLFW_KEY_F11:return VKEY_F11;
	case GLFW_KEY_F12:return VKEY_F12;
	}
	return 0;
}

//monkey key to special monkey char
int BBGlfwGame::KeyToChar( int key ){
	switch( key ){
	case VKEY_BACKSPACE:
	case VKEY_TAB:
	case VKEY_ENTER:
	case VKEY_ESCAPE:
		return key;
	case VKEY_PAGE_UP:
	case VKEY_PAGE_DOWN:
	case VKEY_END:
	case VKEY_HOME:
	case VKEY_LEFT:
	case VKEY_UP:
	case VKEY_RIGHT:
	case VKEY_DOWN:
	case VKEY_INSERT:
		return key | 0x10000;
	case VKEY_DELETE:
		return 127;
	}
	return 0;
}

void BBGlfwGame::OnKey( GLFWwindow *window,int key,int scancode,int action,int mods ){

	key=TransKey( key );
	if( !key ) return;
	
	switch( action ){
	case GLFW_PRESS:
	case GLFW_REPEAT:
		_glfwGame->KeyEvent( BBGameEvent::KeyDown,key );
		if( int chr=KeyToChar( key ) ) _glfwGame->KeyEvent( BBGameEvent::KeyChar,chr );
		break;
	case GLFW_RELEASE:
		_glfwGame->KeyEvent( BBGameEvent::KeyUp,key );
		break;
	}
}

void BBGlfwGame::OnChar( GLFWwindow *window,unsigned int chr ){

	_glfwGame->KeyEvent( BBGameEvent::KeyChar,chr );
}

void BBGlfwGame::OnMouseButton( GLFWwindow *window,int button,int action,int mods ){
	switch( button ){
	case GLFW_MOUSE_BUTTON_LEFT:button=0;break;
	case GLFW_MOUSE_BUTTON_RIGHT:button=1;break;
	case GLFW_MOUSE_BUTTON_MIDDLE:button=2;break;
	default:return;
	}
	double x=0,y=0;
	glfwGetCursorPos( window,&x,&y );
	switch( action ){
	case GLFW_PRESS:
		_glfwGame->MouseEvent( BBGameEvent::MouseDown,button,x,y );
		break;
	case GLFW_RELEASE:
		_glfwGame->MouseEvent( BBGameEvent::MouseUp,button,x,y );
		break;
	}
}

void BBGlfwGame::OnCursorPos( GLFWwindow *window,double x,double y ){
	_glfwGame->MouseEvent( BBGameEvent::MouseMove,-1,x,y );
}

void BBGlfwGame::OnWindowClose( GLFWwindow *window ){
	_glfwGame->KeyEvent( BBGameEvent::KeyDown,0x1b0 );
	_glfwGame->KeyEvent( BBGameEvent::KeyUp,0x1b0 );
}

void BBGlfwGame::OnWindowSize( GLFWwindow *window,int width,int height ){

	_glfwGame->_width=width;
	_glfwGame->_height=height;
	
#if CFG_GLFW_WINDOW_RENDER_WHILE_RESIZING && !__linux
	_glfwGame->RenderGame();
	glfwSwapBuffers( _glfwGame->_window );
	_glfwGame->_nextUpdate=0;
#endif
}

void BBGlfwGame::SetDeviceWindow( int width,int height,int flags ){

	_focus=false;

	if( _window ){
		for( int i=0;i<=GLFW_KEY_LAST;++i ){
			int key=TransKey( i );
			if( key && glfwGetKey( _window,i )==GLFW_PRESS ) KeyEvent( BBGameEvent::KeyUp,key );
		}
		glfwDestroyWindow( _window );
		_window=0;
	}

	bool fullscreen=(flags & 1);
	bool resizable=(flags & 2);
	bool decorated=(flags & 4);
	bool floating=(flags & 8);
	bool depthbuffer=(flags & 16);
	bool doublebuffer=!(flags & 32);
	bool secondmonitor=(flags & 64);

	glfwWindowHint( GLFW_RED_BITS,8 );
	glfwWindowHint( GLFW_GREEN_BITS,8 );
	glfwWindowHint( GLFW_BLUE_BITS,8 );
	glfwWindowHint( GLFW_ALPHA_BITS,0 );
	glfwWindowHint( GLFW_DEPTH_BITS,depthbuffer ? 32 : 0 );
	glfwWindowHint( GLFW_STENCIL_BITS,0 );
	glfwWindowHint( GLFW_RESIZABLE,resizable );
	glfwWindowHint( GLFW_DECORATED,decorated );
	glfwWindowHint( GLFW_FLOATING,floating );
	glfwWindowHint( GLFW_VISIBLE,fullscreen );
	glfwWindowHint( GLFW_DOUBLEBUFFER,doublebuffer );
	glfwWindowHint( GLFW_SAMPLES,CFG_GLFW_WINDOW_SAMPLES );
	glfwWindowHint( GLFW_REFRESH_RATE,60 );
	
	GLFWmonitor *monitor=0;
	if( fullscreen ){
		int monitorid=secondmonitor ? 1 : 0;
		int count=0;
		GLFWmonitor **monitors=glfwGetMonitors( &count );
		if( monitorid>=count ) monitorid=count-1;
		monitor=monitors[monitorid];
	}
	
	_window=glfwCreateWindow( width,height,_STRINGIZE(CFG_GLFW_WINDOW_TITLE),monitor,0 );
	if( !_window ){
		bbPrint( "glfwCreateWindow FAILED!" );
		abort();
	}
	
	_width=width;
	_height=height;
	
	++glfwGraphicsSeq;

	if( !fullscreen ){	
		glfwSetWindowPos( _window,(_desktopMode.width-width)/2,(_desktopMode.height-height)/2 );
		glfwShowWindow( _window );
	}
	
	glfwMakeContextCurrent( _window );
	
	if( _swapInterval>=0 ) glfwSwapInterval( _swapInterval );

#if CFG_OPENGL_INIT_EXTENSIONS
	Init_GL_Exts();
#endif

	glfwSetKeyCallback( _window,OnKey );
	glfwSetCharCallback( _window,OnChar );
	glfwSetMouseButtonCallback( _window,OnMouseButton );
	glfwSetCursorPosCallback( _window,OnCursorPos );
	glfwSetWindowCloseCallback(	_window,OnWindowClose );
	glfwSetWindowSizeCallback(_window,OnWindowSize );
}

void BBGlfwGame::SetSwapInterval( int interval ){

	_swapInterval=interval;
	
	if( _swapInterval>=0 && _window ) glfwSwapInterval( _swapInterval );
}

Array<BBDisplayMode*> BBGlfwGame::GetDisplayModes(){
	int count=0;
	const GLFWvidmode *vmodes=glfwGetVideoModes( glfwGetPrimaryMonitor(),&count );
	Array<BBDisplayMode*> modes( count );
	int n=0;
	for( int i=0;i<count;++i ){
		const GLFWvidmode *vmode=&vmodes[i];
		if( vmode->refreshRate && vmode->refreshRate!=60 ) continue;
		modes[n++]=new BBDisplayMode( vmode->width,vmode->height );
	}
	return modes.Slice(0,n);
}

BBDisplayMode *BBGlfwGame::GetDesktopMode(){ 
	return new BBDisplayMode( _desktopMode.width,_desktopMode.height ); 
}

double BBGlfwGame::GetTime(){
	return glfwGetTime();
}

void BBGlfwGame::Sleep( double time ){
#if _WIN32
	WaitForSingleObject( GetCurrentThread(),(DWORD)( time*1000.0 ) );
#else
	timespec ts,rem;
	ts.tv_sec=floor(time);
	ts.tv_nsec=(time-floor(time))*1000000000.0;
	while( nanosleep( &ts,&rem )==EINTR ){
		ts=rem;
	}
#endif
}

void BBGlfwGame::UpdateEvents(){

	if( _suspended ){
		glfwWaitEvents();
	}else{
		glfwPollEvents();
	}
	if( glfwGetWindowAttrib( _window,GLFW_FOCUSED ) ){
		_focus=true;
		if( _suspended ){
			ResumeGame();
			_nextUpdate=0;
		}
	}else if( glfwGetWindowAttrib( _window,GLFW_ICONIFIED ) || CFG_MOJO_AUTO_SUSPEND_ENABLED ){
		if( _focus && !_suspended ){
			SuspendGame();
			_nextUpdate=0;
		}
	}
}

void BBGlfwGame::Run(){

#if	CFG_GLFW_WINDOW_WIDTH && CFG_GLFW_WINDOW_HEIGHT

	int flags=0;
#if CFG_GLFW_WINDOW_FULLSCREEN
	flags|=1;
#endif
#if CFG_GLFW_WINDOW_RESIZABLE
	flags|=2;
#endif
#if CFG_GLFW_WINDOW_DECORATED
	flags|=4;
#endif
#if CFG_GLFW_WINDOW_FLOATING
	flags|=8;
#endif
#if CFG_OPENGL_DEPTH_BUFFER_ENABLED
	flags|=16;
#endif

	SetDeviceWindow( CFG_GLFW_WINDOW_WIDTH,CFG_GLFW_WINDOW_HEIGHT,flags );

#endif

	StartGame();
	
	while( !glfwWindowShouldClose( _window ) ){
	
		RenderGame();
		
		glfwSwapBuffers( _window );
		
		//Wait for next update
		if( _nextUpdate ){
			double delay=_nextUpdate-GetTime();
			if( delay>0 ) Sleep( delay );
		}
		
		//Update user events
		UpdateEvents();

		//App suspended?		
		if( _suspended ){
			continue;
		}

		//'Go nuts' mode!
		if( !_updateRate ){
			UpdateGame();
			continue;
		}
		
		//Reset update timer?
		if( !_nextUpdate ){
			_nextUpdate=GetTime();
		}
		
		//Catch up updates...
		int i=0;
		for( ;i<4;++i ){
		
			UpdateGame();
			if( !_nextUpdate ) break;
			
			_nextUpdate+=_updatePeriod;
			
			if( _nextUpdate>GetTime() ) break;
		}
		
		if( i==4 ) _nextUpdate=0;
	}
}



//***** monkeygame.h *****

class BBMonkeyGame : public BBGlfwGame{
public:
	static void Main( int args,const char *argv[] );
};

//***** monkeygame.cpp *****

#define _QUOTE(X) #X
#define _STRINGIZE(X) _QUOTE(X)

static void onGlfwError( int err,const char *msg ){
	printf( "GLFW Error: err=%i, msg=%s\n",err,msg );
	fflush( stdout );
}

void BBMonkeyGame::Main( int argc,const char *argv[] ){

	glfwSetErrorCallback( onGlfwError );
	
	if( !glfwInit() ){
		puts( "glfwInit failed" );
		exit( -1 );
	}

	BBMonkeyGame *game=new BBMonkeyGame();
	
	try{
	
		bb_std_main( argc,argv );
		
	}catch( ThrowableObject *ex ){
	
		glfwTerminate();
		
		game->Die( ex );
		
		return;
	}

	if( game->Delegate() ) game->Run();
	
	glfwTerminate();
}


// GLFW mojo runtime.
//
// Copyright 2011 Mark Sibly, all rights reserved.
// No warranty implied; use at your own risk.

//***** gxtkGraphics.h *****

class gxtkSurface;

class gxtkGraphics : public Object{
public:

	enum{
		MAX_VERTS=1024,
		MAX_QUADS=(MAX_VERTS/4)
	};

	int width;
	int height;

	int colorARGB;
	float r,g,b,alpha;
	float ix,iy,jx,jy,tx,ty;
	bool tformed;

	float vertices[MAX_VERTS*5];
	unsigned short quadIndices[MAX_QUADS*6];

	int primType;
	int vertCount;
	gxtkSurface *primSurf;
	
	gxtkGraphics();
	
	void Flush();
	float *Begin( int type,int count,gxtkSurface *surf );
	
	//***** GXTK API *****
	virtual int Width();
	virtual int Height();
	
	virtual int  BeginRender();
	virtual void EndRender();
	virtual void DiscardGraphics();

	virtual gxtkSurface *LoadSurface( String path );
	virtual gxtkSurface *CreateSurface( int width,int height );
	virtual bool LoadSurface__UNSAFE__( gxtkSurface *surface,String path );
	
	virtual int Cls( float r,float g,float b );
	virtual int SetAlpha( float alpha );
	virtual int SetColor( float r,float g,float b );
	virtual int SetBlend( int blend );
	virtual int SetScissor( int x,int y,int w,int h );
	virtual int SetMatrix( float ix,float iy,float jx,float jy,float tx,float ty );
	
	virtual int DrawPoint( float x,float y );
	virtual int DrawRect( float x,float y,float w,float h );
	virtual int DrawLine( float x1,float y1,float x2,float y2 );
	virtual int DrawOval( float x1,float y1,float x2,float y2 );
	virtual int DrawPoly( Array<Float> verts );
	virtual int DrawPoly2( Array<Float> verts,gxtkSurface *surface,int srcx,int srcy );
	virtual int DrawSurface( gxtkSurface *surface,float x,float y );
	virtual int DrawSurface2( gxtkSurface *surface,float x,float y,int srcx,int srcy,int srcw,int srch );
	
	virtual int ReadPixels( Array<int> pixels,int x,int y,int width,int height,int offset,int pitch );
	virtual int WritePixels2( gxtkSurface *surface,Array<int> pixels,int x,int y,int width,int height,int offset,int pitch );
};

class gxtkSurface : public Object{
public:
	unsigned char *data;
	int width;
	int height;
	int depth;
	int format;
	int seq;
	
	GLuint texture;
	float uscale;
	float vscale;
	
	gxtkSurface();
	
	void SetData( unsigned char *data,int width,int height,int depth );
	void SetSubData( int x,int y,int w,int h,unsigned *src,int pitch );
	void Bind();
	
	~gxtkSurface();
	
	//***** GXTK API *****
	virtual int Discard();
	virtual int Width();
	virtual int Height();
	virtual int Loaded();
	virtual void OnUnsafeLoadComplete();
};

//***** gxtkGraphics.cpp *****

#ifndef GL_BGRA
#define GL_BGRA  0x80e1
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812f
#endif

#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

static int Pow2Size( int n ){
	int i=1;
	while( i<n ) i+=i;
	return i;
}

gxtkGraphics::gxtkGraphics(){

	width=height=0;
	vertCount=0;
	
#ifdef _glfw3_h_
	GLFWwindow *window=BBGlfwGame::GlfwGame()->GetGLFWwindow();
	if( window ) glfwGetWindowSize( BBGlfwGame::GlfwGame()->GetGLFWwindow(),&width,&height );
#else
	glfwGetWindowSize( &width,&height );
#endif
	
	if( CFG_OPENGL_GLES20_ENABLED ) return;
	
	for( int i=0;i<MAX_QUADS;++i ){
		quadIndices[i*6  ]=(short)(i*4);
		quadIndices[i*6+1]=(short)(i*4+1);
		quadIndices[i*6+2]=(short)(i*4+2);
		quadIndices[i*6+3]=(short)(i*4);
		quadIndices[i*6+4]=(short)(i*4+2);
		quadIndices[i*6+5]=(short)(i*4+3);
	}
}

void gxtkGraphics::Flush(){
	if( !vertCount ) return;

	if( primSurf ){
		glEnable( GL_TEXTURE_2D );
		primSurf->Bind();
	}
		
	switch( primType ){
	case 1:
		glDrawArrays( GL_POINTS,0,vertCount );
		break;
	case 2:
		glDrawArrays( GL_LINES,0,vertCount );
		break;
	case 3:
		glDrawArrays( GL_TRIANGLES,0,vertCount );
		break;
	case 4:
		glDrawElements( GL_TRIANGLES,vertCount/4*6,GL_UNSIGNED_SHORT,quadIndices );
		break;
	default:
		for( int j=0;j<vertCount;j+=primType ){
			glDrawArrays( GL_TRIANGLE_FAN,j,primType );
		}
		break;
	}

	if( primSurf ){
		glDisable( GL_TEXTURE_2D );
	}

	vertCount=0;
}

float *gxtkGraphics::Begin( int type,int count,gxtkSurface *surf ){
	if( primType!=type || primSurf!=surf || vertCount+count>MAX_VERTS ){
		Flush();
		primType=type;
		primSurf=surf;
	}
	float *vp=vertices+vertCount*5;
	vertCount+=count;
	return vp;
}

//***** GXTK API *****

int gxtkGraphics::Width(){
	return width;
}

int gxtkGraphics::Height(){
	return height;
}

int gxtkGraphics::BeginRender(){

	width=height=0;
#ifdef _glfw3_h_
	glfwGetWindowSize( BBGlfwGame::GlfwGame()->GetGLFWwindow(),&width,&height );
#else
	glfwGetWindowSize( &width,&height );
#endif

#if CFG_OPENGL_GLES20_ENABLED
	return 0;
#else

	glViewport( 0,0,width,height );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0,width,height,0,-1,1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2,GL_FLOAT,20,&vertices[0] );	
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2,GL_FLOAT,20,&vertices[2] );
	
	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4,GL_UNSIGNED_BYTE,20,&vertices[4] );
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE,GL_ONE_MINUS_SRC_ALPHA );
	
	glDisable( GL_TEXTURE_2D );
	
	vertCount=0;
	
	return 1;
	
#endif
}

void gxtkGraphics::EndRender(){
	if( !CFG_OPENGL_GLES20_ENABLED ) Flush();
}

void gxtkGraphics::DiscardGraphics(){
}

int gxtkGraphics::Cls( float r,float g,float b ){
	vertCount=0;

	glClearColor( r/255.0f,g/255.0f,b/255.0f,1 );
	glClear( GL_COLOR_BUFFER_BIT );

	return 0;
}

int gxtkGraphics::SetAlpha( float alpha ){
	this->alpha=alpha;
	
	int a=int(alpha*255);
	
	colorARGB=(a<<24) | (int(b*alpha)<<16) | (int(g*alpha)<<8) | int(r*alpha);
	
	return 0;
}

int gxtkGraphics::SetColor( float r,float g,float b ){
	this->r=r;
	this->g=g;
	this->b=b;

	int a=int(alpha*255);
	
	colorARGB=(a<<24) | (int(b*alpha)<<16) | (int(g*alpha)<<8) | int(r*alpha);
	
	return 0;
}

int gxtkGraphics::SetBlend( int blend ){

	Flush();
	
	switch( blend ){
	case 1:
		glBlendFunc( GL_ONE,GL_ONE );
		break;
	default:
		glBlendFunc( GL_ONE,GL_ONE_MINUS_SRC_ALPHA );
	}

	return 0;
}

int gxtkGraphics::SetScissor( int x,int y,int w,int h ){

	Flush();
	
	if( x!=0 || y!=0 || w!=Width() || h!=Height() ){
		glEnable( GL_SCISSOR_TEST );
		y=Height()-y-h;
		glScissor( x,y,w,h );
	}else{
		glDisable( GL_SCISSOR_TEST );
	}
	return 0;
}

int gxtkGraphics::SetMatrix( float ix,float iy,float jx,float jy,float tx,float ty ){

	tformed=(ix!=1 || iy!=0 || jx!=0 || jy!=1 || tx!=0 || ty!=0);

	this->ix=ix;this->iy=iy;this->jx=jx;this->jy=jy;this->tx=tx;this->ty=ty;

	return 0;
}

int gxtkGraphics::DrawPoint( float x,float y ){

	if( tformed ){
		float px=x;
		x=px * ix + y * jx + tx;
		y=px * iy + y * jy + ty;
	}
	
	float *vp=Begin( 1,1,0 );
	
	vp[0]=x+.5f;vp[1]=y+.5f;(int&)vp[4]=colorARGB;

	return 0;	
}
	
int gxtkGraphics::DrawLine( float x0,float y0,float x1,float y1 ){

	if( tformed ){
		float tx0=x0,tx1=x1;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
	}
	
	float *vp=Begin( 2,2,0 );

	vp[0]=x0+.5f;vp[1]=y0+.5f;(int&)vp[4]=colorARGB;
	vp[5]=x1+.5f;vp[6]=y1+.5f;(int&)vp[9]=colorARGB;
	
	return 0;
}

int gxtkGraphics::DrawRect( float x,float y,float w,float h ){

	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=Begin( 4,4,0 );

	vp[0 ]=x0;vp[1 ]=y0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;(int&)vp[19]=colorARGB;

	return 0;
}

int gxtkGraphics::DrawOval( float x,float y,float w,float h ){
	
	float xr=w/2.0f;
	float yr=h/2.0f;

	int n;
	if( tformed ){
		float dx_x=xr * ix;
		float dx_y=xr * iy;
		float dx=sqrtf( dx_x*dx_x+dx_y*dx_y );
		float dy_x=yr * jx;
		float dy_y=yr * jy;
		float dy=sqrtf( dy_x*dy_x+dy_y*dy_y );
		n=(int)( dx+dy );
	}else{
		n=(int)( fabs( xr )+fabs( yr ) );
	}
	
	if( n<12 ){
		n=12;
	}else if( n>MAX_VERTS ){
		n=MAX_VERTS;
	}else{
		n&=~3;
	}

	float x0=x+xr,y0=y+yr;
	
	float *vp=Begin( n,n,0 );

	for( int i=0;i<n;++i ){
	
		float th=i * 6.28318531f / n;

		float px=x0+cosf( th ) * xr;
		float py=y0-sinf( th ) * yr;
		
		if( tformed ){
			float ppx=px;
			px=ppx * ix + py * jx + tx;
			py=ppx * iy + py * jy + ty;
		}
		
		vp[0]=px;vp[1]=py;(int&)vp[4]=colorARGB;
		vp+=5;
	}
	
	return 0;
}

int gxtkGraphics::DrawPoly( Array<Float> verts ){

	int n=verts.Length()/2;
	if( n<1 || n>MAX_VERTS ) return 0;
	
	float *vp=Begin( n,n,0 );
	
	for( int i=0;i<n;++i ){
		int j=i*2;
		if( tformed ){
			vp[0]=verts[j] * ix + verts[j+1] * jx + tx;
			vp[1]=verts[j] * iy + verts[j+1] * jy + ty;
		}else{
			vp[0]=verts[j];
			vp[1]=verts[j+1];
		}
		(int&)vp[4]=colorARGB;
		vp+=5;
	}

	return 0;
}

int gxtkGraphics::DrawPoly2( Array<Float> verts,gxtkSurface *surface,int srcx,int srcy ){

	int n=verts.Length()/4;
	if( n<1 || n>MAX_VERTS ) return 0;
		
	float *vp=Begin( n,n,surface );
	
	for( int i=0;i<n;++i ){
		int j=i*4;
		if( tformed ){
			vp[0]=verts[j] * ix + verts[j+1] * jx + tx;
			vp[1]=verts[j] * iy + verts[j+1] * jy + ty;
		}else{
			vp[0]=verts[j];
			vp[1]=verts[j+1];
		}
		vp[2]=(srcx+verts[j+2])*surface->uscale;
		vp[3]=(srcy+verts[j+3])*surface->vscale;
		(int&)vp[4]=colorARGB;
		vp+=5;
	}
	
	return 0;
}

int gxtkGraphics::DrawSurface( gxtkSurface *surf,float x,float y ){
	
	float w=surf->Width();
	float h=surf->Height();
	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;
	float u0=0,u1=w*surf->uscale;
	float v0=0,v1=h*surf->vscale;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=Begin( 4,4,surf );
	
	vp[0 ]=x0;vp[1 ]=y0;vp[2 ]=u0;vp[3 ]=v0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;vp[7 ]=u1;vp[8 ]=v0;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;vp[12]=u1;vp[13]=v1;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;vp[17]=u0;vp[18]=v1;(int&)vp[19]=colorARGB;
	
	return 0;
}

int gxtkGraphics::DrawSurface2( gxtkSurface *surf,float x,float y,int srcx,int srcy,int srcw,int srch ){
	
	float w=srcw;
	float h=srch;
	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;
	float u0=srcx*surf->uscale,u1=(srcx+srcw)*surf->uscale;
	float v0=srcy*surf->vscale,v1=(srcy+srch)*surf->vscale;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=Begin( 4,4,surf );
	
	vp[0 ]=x0;vp[1 ]=y0;vp[2 ]=u0;vp[3 ]=v0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;vp[7 ]=u1;vp[8 ]=v0;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;vp[12]=u1;vp[13]=v1;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;vp[17]=u0;vp[18]=v1;(int&)vp[19]=colorARGB;
	
	return 0;
}
	
int gxtkGraphics::ReadPixels( Array<int> pixels,int x,int y,int width,int height,int offset,int pitch ){

	Flush();

	unsigned *p=(unsigned*)malloc(width*height*4);

	glReadPixels( x,this->height-y-height,width,height,GL_BGRA,GL_UNSIGNED_BYTE,p );
	
	for( int py=0;py<height;++py ){
		memcpy( &pixels[offset+py*pitch],&p[(height-py-1)*width],width*4 );
	}
	
	free( p );
	
	return 0;
}

int gxtkGraphics::WritePixels2( gxtkSurface *surface,Array<int> pixels,int x,int y,int width,int height,int offset,int pitch ){

	surface->SetSubData( x,y,width,height,(unsigned*)&pixels[offset],pitch );
	
	return 0;
}

//***** gxtkSurface *****

gxtkSurface::gxtkSurface():data(0),width(0),height(0),depth(0),format(0),seq(-1),texture(0),uscale(0),vscale(0){
}

gxtkSurface::~gxtkSurface(){
	Discard();
}

int gxtkSurface::Discard(){
	if( seq==glfwGraphicsSeq ){
		glDeleteTextures( 1,&texture );
		seq=-1;
	}
	if( data ){
		free( data );
		data=0;
	}
	return 0;
}

int gxtkSurface::Width(){
	return width;
}

int gxtkSurface::Height(){
	return height;
}

int gxtkSurface::Loaded(){
	return 1;
}

//Careful! Can't call any GL here as it may be executing off-thread.
//
void gxtkSurface::SetData( unsigned char *data,int width,int height,int depth ){

	this->data=data;
	this->width=width;
	this->height=height;
	this->depth=depth;
	
	unsigned char *p=data;
	int n=width*height;
	
	switch( depth ){
	case 1:
		format=GL_LUMINANCE;
		break;
	case 2:
		format=GL_LUMINANCE_ALPHA;
		if( data ){
			while( n-- ){	//premultiply alpha
				p[0]=p[0]*p[1]/255;
				p+=2;
			}
		}
		break;
	case 3:
		format=GL_RGB;
		break;
	case 4:
		format=GL_RGBA;
		if( data ){
			while( n-- ){	//premultiply alpha
				p[0]=p[0]*p[3]/255;
				p[1]=p[1]*p[3]/255;
				p[2]=p[2]*p[3]/255;
				p+=4;
			}
		}
		break;
	}
}

void gxtkSurface::SetSubData( int x,int y,int w,int h,unsigned *src,int pitch ){
	if( format!=GL_RGBA ) return;
	
	if( !data ) data=(unsigned char*)malloc( width*height*4 );
	
	unsigned *dst=(unsigned*)data+y*width+x;
	
	for( int py=0;py<h;++py ){
		unsigned *d=dst+py*width;
		unsigned *s=src+py*pitch;
		for( int px=0;px<w;++px ){
			unsigned p=*s++;
			unsigned a=p>>24;
			*d++=(a<<24) | ((p>>0&0xff)*a/255<<16) | ((p>>8&0xff)*a/255<<8) | ((p>>16&0xff)*a/255);
		}
	}
	
	if( seq==glfwGraphicsSeq ){
		glBindTexture( GL_TEXTURE_2D,texture );
		glPixelStorei( GL_UNPACK_ALIGNMENT,1 );
		if( width==pitch ){
			glTexSubImage2D( GL_TEXTURE_2D,0,x,y,w,h,format,GL_UNSIGNED_BYTE,dst );
		}else{
			for( int py=0;py<h;++py ){
				glTexSubImage2D( GL_TEXTURE_2D,0,x,y+py,w,1,format,GL_UNSIGNED_BYTE,dst+py*width );
			}
		}
	}
}

void gxtkSurface::Bind(){

	if( !glfwGraphicsSeq ) return;
	
	if( seq==glfwGraphicsSeq ){
		glBindTexture( GL_TEXTURE_2D,texture );
		return;
	}
	
	seq=glfwGraphicsSeq;
	
	glGenTextures( 1,&texture );
	glBindTexture( GL_TEXTURE_2D,texture );
	
	if( CFG_MOJO_IMAGE_FILTERING_ENABLED ){
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
	}else{
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST );
	}

	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE );

	int texwidth=width;
	int texheight=height;
	
	glTexImage2D( GL_TEXTURE_2D,0,format,texwidth,texheight,0,format,GL_UNSIGNED_BYTE,0 );
	if( glGetError()!=GL_NO_ERROR ){
		texwidth=Pow2Size( width );
		texheight=Pow2Size( height );
		glTexImage2D( GL_TEXTURE_2D,0,format,texwidth,texheight,0,format,GL_UNSIGNED_BYTE,0 );
	}
	
	uscale=1.0/texwidth;
	vscale=1.0/texheight;
	
	if( data ){
		glPixelStorei( GL_UNPACK_ALIGNMENT,1 );
		glTexSubImage2D( GL_TEXTURE_2D,0,0,0,width,height,format,GL_UNSIGNED_BYTE,data );
	}
}

void gxtkSurface::OnUnsafeLoadComplete(){
	Bind();
}

bool gxtkGraphics::LoadSurface__UNSAFE__( gxtkSurface *surface,String path ){

	int width,height,depth;
	unsigned char *data=BBGlfwGame::GlfwGame()->LoadImageData( path,&width,&height,&depth );
	if( !data ) return false;
	
	surface->SetData( data,width,height,depth );
	return true;
}

gxtkSurface *gxtkGraphics::LoadSurface( String path ){
	gxtkSurface *surf=new gxtkSurface();
	if( !LoadSurface__UNSAFE__( surf,path ) ) return 0;
	surf->Bind();
	return surf;
}

gxtkSurface *gxtkGraphics::CreateSurface( int width,int height ){
	gxtkSurface *surf=new gxtkSurface();
	surf->SetData( 0,width,height,4 );
	surf->Bind();
	return surf;
}

//***** gxtkAudio.h *****

class gxtkSample;

class gxtkChannel{
public:
	ALuint source;
	gxtkSample *sample;
	int flags;
	int state;
	
	int AL_Source();
};

class gxtkAudio : public Object{
public:
	static gxtkAudio *audio;
	
	ALCdevice *alcDevice;
	ALCcontext *alcContext;
	gxtkChannel channels[33];

	gxtkAudio();

	virtual void mark();

	//***** GXTK API *****
	virtual int Suspend();
	virtual int Resume();

	virtual gxtkSample *LoadSample( String path );
	virtual bool LoadSample__UNSAFE__( gxtkSample *sample,String path );
	
	virtual int PlaySample( gxtkSample *sample,int channel,int flags );

	virtual int StopChannel( int channel );
	virtual int PauseChannel( int channel );
	virtual int ResumeChannel( int channel );
	virtual int ChannelState( int channel );
	virtual int SetVolume( int channel,float volume );
	virtual int SetPan( int channel,float pan );
	virtual int SetRate( int channel,float rate );
	
	virtual int PlayMusic( String path,int flags );
	virtual int StopMusic();
	virtual int PauseMusic();
	virtual int ResumeMusic();
	virtual int MusicState();
	virtual int SetMusicVolume( float volume );
};

class gxtkSample : public Object{
public:
	ALuint al_buffer;

	gxtkSample();
	gxtkSample( ALuint buf );
	~gxtkSample();
	
	void SetBuffer( ALuint buf );
	
	//***** GXTK API *****
	virtual int Discard();
};

//***** gxtkAudio.cpp *****

gxtkAudio *gxtkAudio::audio;

static std::vector<ALuint> discarded;

static void FlushDiscarded(){

	if( !discarded.size() ) return;
	
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&gxtkAudio::audio->channels[i];
		if( chan->state ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_STOPPED ) alSourcei( chan->source,AL_BUFFER,0 );
		}
	}
	
	std::vector<ALuint> out;
	
	for( int i=0;i<discarded.size();++i ){
		ALuint buf=discarded[i];
		alDeleteBuffers( 1,&buf );
		ALenum err=alGetError();
		if( err==AL_NO_ERROR ){
//			printf( "alDeleteBuffers OK!\n" );fflush( stdout );
		}else{
//			printf( "alDeleteBuffers failed...\n" );fflush( stdout );
			out.push_back( buf );
		}
	}
	discarded=out;
}

int gxtkChannel::AL_Source(){
	if( source ) return source;

	alGetError();
	alGenSources( 1,&source );
	if( alGetError()==AL_NO_ERROR ) return source;
	
	//couldn't create source...steal a free source...?
	//
	source=0;
	for( int i=0;i<32;++i ){
		gxtkChannel *chan=&gxtkAudio::audio->channels[i];
		if( !chan->source || gxtkAudio::audio->ChannelState( i ) ) continue;
//		puts( "Stealing source!" );
		source=chan->source;
		chan->source=0;
		break;
	}
	return source;
}

gxtkAudio::gxtkAudio(){

	audio=this;
	
	alcDevice=alcOpenDevice( 0 );
	if( !alcDevice ){
		alcDevice=alcOpenDevice( "Generic Hardware" );
		if( !alcDevice ) alcDevice=alcOpenDevice( "Generic Software" );
	}

//	bbPrint( "opening openal device" );
	if( alcDevice ){
		if( (alcContext=alcCreateContext( alcDevice,0 )) ){
			if( (alcMakeContextCurrent( alcContext )) ){
				//alc all go!
			}else{
				bbPrint( "OpenAl error: alcMakeContextCurrent failed" );
			}
		}else{
			bbPrint( "OpenAl error: alcCreateContext failed" );
		}
	}else{
		bbPrint( "OpenAl error: alcOpenDevice failed" );
	}

	alDistanceModel( AL_NONE );
	
	memset( channels,0,sizeof(channels) );

	channels[32].AL_Source();
}

void gxtkAudio::mark(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state!=0 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state!=AL_STOPPED ) gc_mark( chan->sample );
		}
	}
}

int gxtkAudio::Suspend(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state==1 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_PLAYING ) alSourcePause( chan->source );
		}
	}
	return 0;
}

int gxtkAudio::Resume(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state==1 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_PAUSED ) alSourcePlay( chan->source );
		}
	}
	return 0;
}

bool gxtkAudio::LoadSample__UNSAFE__( gxtkSample *sample,String path ){

	int length=0;
	int channels=0;
	int format=0;
	int hertz=0;
	unsigned char *data=BBGlfwGame::GlfwGame()->LoadAudioData( path,&length,&channels,&format,&hertz );
	if( !data ) return false;
	
	int al_format=0;
	if( format==1 && channels==1 ){
		al_format=AL_FORMAT_MONO8;
	}else if( format==1 && channels==2 ){
		al_format=AL_FORMAT_STEREO8;
	}else if( format==2 && channels==1 ){
		al_format=AL_FORMAT_MONO16;
	}else if( format==2 && channels==2 ){
		al_format=AL_FORMAT_STEREO16;
	}
	
	int size=length*channels*format;
	
	ALuint al_buffer;
	alGenBuffers( 1,&al_buffer );
	alBufferData( al_buffer,al_format,data,size,hertz );
	free( data );
	
	sample->SetBuffer( al_buffer );
	return true;
}

gxtkSample *gxtkAudio::LoadSample( String path ){
	FlushDiscarded();
	gxtkSample *sample=new gxtkSample();
	if( !LoadSample__UNSAFE__( sample,path ) ) return 0;
	return sample;
}

int gxtkAudio::PlaySample( gxtkSample *sample,int channel,int flags ){

	FlushDiscarded();
	
	gxtkChannel *chan=&channels[channel];
	
	if( !chan->AL_Source() ) return -1;
	
	alSourceStop( chan->source );
	alSourcei( chan->source,AL_BUFFER,sample->al_buffer );
	alSourcei( chan->source,AL_LOOPING,flags ? 1 : 0 );
	alSourcePlay( chan->source );
	
	gc_assign( chan->sample,sample );

	chan->flags=flags;
	chan->state=1;

	return 0;
}

int gxtkAudio::StopChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state!=0 ){
		alSourceStop( chan->source );
		chan->state=0;
	}
	return 0;
}

int gxtkAudio::PauseChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state==1 ){
		int state=0;
		alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
		if( state==AL_STOPPED ){
			chan->state=0;
		}else{
			alSourcePause( chan->source );
			chan->state=2;
		}
	}
	return 0;
}

int gxtkAudio::ResumeChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state==2 ){
		alSourcePlay( chan->source );
		chan->state=1;
	}
	return 0;
}

int gxtkAudio::ChannelState( int channel ){
	gxtkChannel *chan=&channels[channel];
	
	if( chan->state==1 ){
		int state=0;
		alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
		if( state==AL_STOPPED ) chan->state=0;
	}
	return chan->state;
}

int gxtkAudio::SetVolume( int channel,float volume ){
	gxtkChannel *chan=&channels[channel];

	alSourcef( chan->AL_Source(),AL_GAIN,volume );
	return 0;
}

int gxtkAudio::SetPan( int channel,float pan ){
	gxtkChannel *chan=&channels[channel];
	
	float x=sinf( pan ),y=0,z=-cosf( pan );
	alSource3f( chan->AL_Source(),AL_POSITION,x,y,z );
	return 0;
}

int gxtkAudio::SetRate( int channel,float rate ){
	gxtkChannel *chan=&channels[channel];

	alSourcef( chan->AL_Source(),AL_PITCH,rate );
	return 0;
}

int gxtkAudio::PlayMusic( String path,int flags ){
	StopMusic();
	
	gxtkSample *music=LoadSample( path );
	if( !music ) return -1;
	
	PlaySample( music,32,flags );
	return 0;
}

int gxtkAudio::StopMusic(){
	StopChannel( 32 );
	return 0;
}

int gxtkAudio::PauseMusic(){
	PauseChannel( 32 );
	return 0;
}

int gxtkAudio::ResumeMusic(){
	ResumeChannel( 32 );
	return 0;
}

int gxtkAudio::MusicState(){
	return ChannelState( 32 );
}

int gxtkAudio::SetMusicVolume( float volume ){
	SetVolume( 32,volume );
	return 0;
}

gxtkSample::gxtkSample():
al_buffer(0){
}

gxtkSample::gxtkSample( ALuint buf ):
al_buffer(buf){
}

gxtkSample::~gxtkSample(){
	Discard();
}

void gxtkSample::SetBuffer( ALuint buf ){
	al_buffer=buf;
}

int gxtkSample::Discard(){
	if( al_buffer ){
		discarded.push_back( al_buffer );
		al_buffer=0;
	}
	return 0;
}


// ***** thread.h *****

#if __cplusplus_winrt

using namespace Windows::System::Threading;

#endif

class BBThread : public Object{
public:
	Object *result;
	
	BBThread();
	
	virtual void Start();
	virtual bool IsRunning();
	
	virtual Object *Result();
	virtual void SetResult( Object *result );
	
	static  String Strdup( const String &str );
	
	virtual void Run__UNSAFE__();
	
	
private:

	enum{
		INIT=0,
		RUNNING=1,
		FINISHED=2
	};

	
	int _state;
	Object *_result;
	
#if __cplusplus_winrt

	friend class Launcher;

	class Launcher{
	
		friend class BBThread;
		BBThread *_thread;
		
		Launcher( BBThread *thread ):_thread(thread){
		}
		
		public:
		
		void operator()( IAsyncAction ^operation ){
			_thread->Run__UNSAFE__();
			_thread->_state=FINISHED;
		} 
	};
	
#elif _WIN32

	static DWORD WINAPI run( void *p );
	
#else

	static void *run( void *p );
	
#endif

};

// ***** thread.cpp *****

BBThread::BBThread():_state( INIT ),_result( 0 ){
}

bool BBThread::IsRunning(){
	return _state==RUNNING;
}

Object *BBThread::Result(){
	return _result;
}

void BBThread::SetResult( Object *result ){
	_result=result;
}

String BBThread::Strdup( const String &str ){
	return str.Copy();
}

void BBThread::Run__UNSAFE__(){
}

#if __cplusplus_winrt

void BBThread::Start(){
	if( _state==RUNNING ) return;
	
	_result=0;
	_state=RUNNING;
	
	Launcher launcher( this );
	
	auto handler=ref new WorkItemHandler( launcher );
	
	ThreadPool::RunAsync( handler );
}

#elif _WIN32

void BBThread::Start(){
	if( _state==RUNNING ) return;
	
	_result=0;
	_state=RUNNING;
	
	DWORD _id;
	HANDLE _handle;

	if( _handle=CreateThread( 0,0,run,this,0,&_id ) ){
		CloseHandle( _handle );
		return;
	}
	
	puts( "CreateThread failed!" );
	exit( -1 );
}

DWORD WINAPI BBThread::run( void *p ){
	BBThread *thread=(BBThread*)p;

	thread->Run__UNSAFE__();
	
	thread->_state=FINISHED;
	return 0;
}

#else

void BBThread::Start(){
	if( _state==RUNNING ) return;
	
	_result=0;
	_state=RUNNING;
	
	pthread_t _handle;
	
	if( !pthread_create( &_handle,0,run,this ) ){
		pthread_detach( _handle );
		return;
	}
	
	puts( "pthread_create failed!" );
	exit( -1 );
}

void *BBThread::run( void *p ){
	BBThread *thread=(BBThread*)p;

	thread->Run__UNSAFE__();

	thread->_state=FINISHED;
	return 0;
}

#endif

class c_App;
class c_Game;
class c_GameDelegate;
class c_Image;
class c_GraphicsContext;
class c_Frame;
class c_InputDevice;
class c_JoyState;
class c_DisplayMode;
class c_Map;
class c_IntMap;
class c_Stack;
class c_Node;
class c_BBGameEvent;
class c_Player;
class c_Sound;
class c_App : public Object{
	public:
	c_App();
	c_App* m_new();
	int p_OnResize();
	virtual int p_OnCreate();
	int p_OnSuspend();
	int p_OnResume();
	virtual int p_OnUpdate();
	int p_OnLoading();
	virtual int p_OnRender();
	int p_OnClose();
	int p_OnBack();
	void mark();
	String debug();
};
String dbg_type(c_App**p){return "App";}
class c_Game : public c_App{
	public:
	c_Player* m_p1;
	c_Sound* m_music;
	c_Game();
	c_Game* m_new();
	int p_DrawSomewhere();
	int p_OnCreate();
	int p_OnUpdate();
	int p_OnRender();
	void mark();
	String debug();
};
String dbg_type(c_Game**p){return "Game";}
extern c_App* bb_app__app;
class c_GameDelegate : public BBGameDelegate{
	public:
	gxtkGraphics* m__graphics;
	gxtkAudio* m__audio;
	c_InputDevice* m__input;
	c_GameDelegate();
	c_GameDelegate* m_new();
	void StartGame();
	void SuspendGame();
	void ResumeGame();
	void UpdateGame();
	void RenderGame();
	void KeyEvent(int,int);
	void MouseEvent(int,int,Float,Float);
	void TouchEvent(int,int,Float,Float);
	void MotionEvent(int,int,Float,Float,Float);
	void DiscardGraphics();
	void mark();
	String debug();
};
String dbg_type(c_GameDelegate**p){return "GameDelegate";}
extern c_GameDelegate* bb_app__delegate;
extern BBGame* bb_app__game;
int bbMain();
extern gxtkGraphics* bb_graphics_device;
int bb_graphics_SetGraphicsDevice(gxtkGraphics*);
class c_Image : public Object{
	public:
	gxtkSurface* m_surface;
	int m_width;
	int m_height;
	Array<c_Frame* > m_frames;
	int m_flags;
	Float m_tx;
	Float m_ty;
	c_Image* m_source;
	c_Image();
	static int m_DefaultFlags;
	c_Image* m_new();
	int p_SetHandle(Float,Float);
	int p_ApplyFlags(int);
	c_Image* p_Init(gxtkSurface*,int,int);
	c_Image* p_Init2(gxtkSurface*,int,int,int,int,int,int,c_Image*,int,int,int,int);
	void mark();
	String debug();
};
String dbg_type(c_Image**p){return "Image";}
class c_GraphicsContext : public Object{
	public:
	c_Image* m_defaultFont;
	c_Image* m_font;
	int m_firstChar;
	int m_matrixSp;
	Float m_ix;
	Float m_iy;
	Float m_jx;
	Float m_jy;
	Float m_tx;
	Float m_ty;
	int m_tformed;
	int m_matDirty;
	Float m_color_r;
	Float m_color_g;
	Float m_color_b;
	Float m_alpha;
	int m_blend;
	Float m_scissor_x;
	Float m_scissor_y;
	Float m_scissor_width;
	Float m_scissor_height;
	Array<Float > m_matrixStack;
	c_GraphicsContext();
	c_GraphicsContext* m_new();
	int p_Validate();
	void mark();
	String debug();
};
String dbg_type(c_GraphicsContext**p){return "GraphicsContext";}
extern c_GraphicsContext* bb_graphics_context;
String bb_data_FixDataPath(String);
class c_Frame : public Object{
	public:
	int m_x;
	int m_y;
	c_Frame();
	c_Frame* m_new(int,int);
	c_Frame* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Frame**p){return "Frame";}
c_Image* bb_graphics_LoadImage(String,int,int);
c_Image* bb_graphics_LoadImage2(String,int,int,int,int);
int bb_graphics_SetFont(c_Image*,int);
extern gxtkAudio* bb_audio_device;
int bb_audio_SetAudioDevice(gxtkAudio*);
class c_InputDevice : public Object{
	public:
	Array<c_JoyState* > m__joyStates;
	Array<bool > m__keyDown;
	int m__keyHitPut;
	Array<int > m__keyHitQueue;
	Array<int > m__keyHit;
	int m__charGet;
	int m__charPut;
	Array<int > m__charQueue;
	Float m__mouseX;
	Float m__mouseY;
	Array<Float > m__touchX;
	Array<Float > m__touchY;
	Float m__accelX;
	Float m__accelY;
	Float m__accelZ;
	c_InputDevice();
	c_InputDevice* m_new();
	void p_PutKeyHit(int);
	void p_BeginUpdate();
	void p_EndUpdate();
	void p_KeyEvent(int,int);
	void p_MouseEvent(int,int,Float,Float);
	void p_TouchEvent(int,int,Float,Float);
	void p_MotionEvent(int,int,Float,Float,Float);
	bool p_KeyDown(int);
	void mark();
	String debug();
};
String dbg_type(c_InputDevice**p){return "InputDevice";}
class c_JoyState : public Object{
	public:
	Array<Float > m_joyx;
	Array<Float > m_joyy;
	Array<Float > m_joyz;
	Array<bool > m_buttons;
	c_JoyState();
	c_JoyState* m_new();
	void mark();
	String debug();
};
String dbg_type(c_JoyState**p){return "JoyState";}
extern c_InputDevice* bb_input_device;
int bb_input_SetInputDevice(c_InputDevice*);
extern int bb_app__devWidth;
extern int bb_app__devHeight;
void bb_app_ValidateDeviceWindow(bool);
class c_DisplayMode : public Object{
	public:
	int m__width;
	int m__height;
	c_DisplayMode();
	c_DisplayMode* m_new(int,int);
	c_DisplayMode* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_DisplayMode**p){return "DisplayMode";}
class c_Map : public Object{
	public:
	c_Node* m_root;
	c_Map();
	c_Map* m_new();
	virtual int p_Compare(int,int)=0;
	c_Node* p_FindNode(int);
	bool p_Contains(int);
	int p_RotateLeft(c_Node*);
	int p_RotateRight(c_Node*);
	int p_InsertFixup(c_Node*);
	bool p_Set(int,c_DisplayMode*);
	bool p_Insert(int,c_DisplayMode*);
	void mark();
	String debug();
};
String dbg_type(c_Map**p){return "Map";}
class c_IntMap : public c_Map{
	public:
	c_IntMap();
	c_IntMap* m_new();
	int p_Compare(int,int);
	void mark();
	String debug();
};
String dbg_type(c_IntMap**p){return "IntMap";}
class c_Stack : public Object{
	public:
	Array<c_DisplayMode* > m_data;
	int m_length;
	c_Stack();
	c_Stack* m_new();
	c_Stack* m_new2(Array<c_DisplayMode* >);
	void p_Push(c_DisplayMode*);
	void p_Push2(Array<c_DisplayMode* >,int,int);
	void p_Push3(Array<c_DisplayMode* >,int);
	Array<c_DisplayMode* > p_ToArray();
	void mark();
	String debug();
};
String dbg_type(c_Stack**p){return "Stack";}
class c_Node : public Object{
	public:
	int m_key;
	c_Node* m_right;
	c_Node* m_left;
	c_DisplayMode* m_value;
	int m_color;
	c_Node* m_parent;
	c_Node();
	c_Node* m_new(int,c_DisplayMode*,int,c_Node*);
	c_Node* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Node**p){return "Node";}
extern Array<c_DisplayMode* > bb_app__displayModes;
extern c_DisplayMode* bb_app__desktopMode;
int bb_app_DeviceWidth();
int bb_app_DeviceHeight();
void bb_app_EnumDisplayModes();
extern gxtkGraphics* bb_graphics_renderDevice;
int bb_graphics_SetMatrix(Float,Float,Float,Float,Float,Float);
int bb_graphics_SetMatrix2(Array<Float >);
int bb_graphics_SetColor(Float,Float,Float);
int bb_graphics_SetAlpha(Float);
int bb_graphics_SetBlend(int);
int bb_graphics_SetScissor(Float,Float,Float,Float);
int bb_graphics_BeginRender();
int bb_graphics_EndRender();
class c_BBGameEvent : public Object{
	public:
	c_BBGameEvent();
	void mark();
	String debug();
};
String dbg_type(c_BBGameEvent**p){return "BBGameEvent";}
void bb_app_EndApp();
class c_Player : public Object{
	public:
	c_Image* m_image;
	int m_x;
	int m_y;
	int m_xVel;
	int m_yVel;
	c_Player();
	c_Player* m_new(c_Image*,int,int);
	c_Player* m_new2();
	int p_Update();
	int p_Draw();
	void mark();
	String debug();
};
String dbg_type(c_Player**p){return "Player";}
class c_Sound : public Object{
	public:
	gxtkSample* m_sample;
	c_Sound();
	c_Sound* m_new(gxtkSample*);
	c_Sound* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Sound**p){return "Sound";}
c_Sound* bb_audio_LoadSound(String);
int bb_audio_SetChannelVolume(int,Float);
int bb_audio_PlaySound(c_Sound*,int,int);
extern int bb_app__updateRate;
void bb_app_SetUpdateRate(int);
int bb_input_KeyDown(int);
int bb_graphics_DebugRenderDevice();
int bb_graphics_Cls(Float,Float,Float);
int bb_graphics_DrawImage(c_Image*,Float,Float,int);
int bb_graphics_PushMatrix();
int bb_graphics_Transform(Float,Float,Float,Float,Float,Float);
int bb_graphics_Transform2(Array<Float >);
int bb_graphics_Translate(Float,Float);
int bb_graphics_Rotate(Float);
int bb_graphics_Scale(Float,Float);
int bb_graphics_PopMatrix();
int bb_graphics_DrawImage2(c_Image*,Float,Float,Float,Float,Float,int);
void gc_mark( BBGame *p ){}
String dbg_type( BBGame **p ){ return "BBGame"; }
String dbg_value( BBGame **p ){ return dbg_ptr_value( *p ); }
c_App::c_App(){
}
c_App* c_App::m_new(){
	DBG_ENTER("App.new")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<152>");
	if((bb_app__app)!=0){
		DBG_BLOCK();
		bbError(String(L"App has already been created",28));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<153>");
	gc_assign(bb_app__app,this);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<154>");
	gc_assign(bb_app__delegate,(new c_GameDelegate)->m_new());
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<155>");
	bb_app__game->SetDelegate(bb_app__delegate);
	return this;
}
int c_App::p_OnResize(){
	DBG_ENTER("App.OnResize")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnCreate(){
	DBG_ENTER("App.OnCreate")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnSuspend(){
	DBG_ENTER("App.OnSuspend")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnResume(){
	DBG_ENTER("App.OnResume")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnUpdate(){
	DBG_ENTER("App.OnUpdate")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnLoading(){
	DBG_ENTER("App.OnLoading")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnRender(){
	DBG_ENTER("App.OnRender")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnClose(){
	DBG_ENTER("App.OnClose")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<177>");
	bb_app_EndApp();
	return 0;
}
int c_App::p_OnBack(){
	DBG_ENTER("App.OnBack")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<181>");
	p_OnClose();
	return 0;
}
void c_App::mark(){
	Object::mark();
}
String c_App::debug(){
	String t="(App)\n";
	return t;
}
c_Game::c_Game(){
	m_p1=0;
	m_music=0;
}
c_Game* c_Game::m_new(){
	DBG_ENTER("Game.new")
	c_Game *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<5>");
	c_App::m_new();
	return this;
}
int c_Game::p_DrawSomewhere(){
	DBG_ENTER("Game.DrawSomewhere")
	c_Game *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<29>");
	bbPrint(String(L"Console",7));
	return 0;
}
int c_Game::p_OnCreate(){
	DBG_ENTER("Game.OnCreate")
	c_Game *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<10>");
	c_Image* t_img=bb_graphics_LoadImage(String(L"rockstar.png",12),1,c_Image::m_DefaultFlags);
	DBG_LOCAL(t_img,"img")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<11>");
	gc_assign(m_p1,(new c_Player)->m_new(t_img,100,100));
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<12>");
	gc_assign(m_music,bb_audio_LoadSound(String(L"Tetris.mp3",10)));
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<13>");
	bb_audio_SetChannelVolume(1,FLOAT(100.0));
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<14>");
	bb_audio_PlaySound(m_music,1,0);
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<15>");
	bb_app_SetUpdateRate(60);
	return 0;
}
int c_Game::p_OnUpdate(){
	DBG_ENTER("Game.OnUpdate")
	c_Game *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<19>");
	m_p1->p_Update();
	return 0;
}
int c_Game::p_OnRender(){
	DBG_ENTER("Game.OnRender")
	c_Game *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<23>");
	bb_graphics_Cls(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<24>");
	m_p1->p_Draw();
	return 0;
}
void c_Game::mark(){
	c_App::mark();
	gc_mark_q(m_p1);
	gc_mark_q(m_music);
}
String c_Game::debug(){
	String t="(Game)\n";
	t=c_App::debug()+t;
	t+=dbg_decl("p1",&m_p1);
	t+=dbg_decl("music",&m_music);
	return t;
}
c_App* bb_app__app;
c_GameDelegate::c_GameDelegate(){
	m__graphics=0;
	m__audio=0;
	m__input=0;
}
c_GameDelegate* c_GameDelegate::m_new(){
	DBG_ENTER("GameDelegate.new")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<65>");
	return this;
}
void c_GameDelegate::StartGame(){
	DBG_ENTER("GameDelegate.StartGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<75>");
	gc_assign(m__graphics,(new gxtkGraphics));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<76>");
	bb_graphics_SetGraphicsDevice(m__graphics);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<77>");
	bb_graphics_SetFont(0,32);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<79>");
	gc_assign(m__audio,(new gxtkAudio));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<80>");
	bb_audio_SetAudioDevice(m__audio);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<82>");
	gc_assign(m__input,(new c_InputDevice)->m_new());
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<83>");
	bb_input_SetInputDevice(m__input);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<85>");
	bb_app_ValidateDeviceWindow(false);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<87>");
	bb_app_EnumDisplayModes();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<89>");
	bb_app__app->p_OnCreate();
}
void c_GameDelegate::SuspendGame(){
	DBG_ENTER("GameDelegate.SuspendGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<93>");
	bb_app__app->p_OnSuspend();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<94>");
	m__audio->Suspend();
}
void c_GameDelegate::ResumeGame(){
	DBG_ENTER("GameDelegate.ResumeGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<98>");
	m__audio->Resume();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<99>");
	bb_app__app->p_OnResume();
}
void c_GameDelegate::UpdateGame(){
	DBG_ENTER("GameDelegate.UpdateGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<103>");
	bb_app_ValidateDeviceWindow(true);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<104>");
	m__input->p_BeginUpdate();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<105>");
	bb_app__app->p_OnUpdate();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<106>");
	m__input->p_EndUpdate();
}
void c_GameDelegate::RenderGame(){
	DBG_ENTER("GameDelegate.RenderGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<110>");
	bb_app_ValidateDeviceWindow(true);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<111>");
	int t_mode=m__graphics->BeginRender();
	DBG_LOCAL(t_mode,"mode")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<112>");
	if((t_mode)!=0){
		DBG_BLOCK();
		bb_graphics_BeginRender();
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<113>");
	if(t_mode==2){
		DBG_BLOCK();
		bb_app__app->p_OnLoading();
	}else{
		DBG_BLOCK();
		bb_app__app->p_OnRender();
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<114>");
	if((t_mode)!=0){
		DBG_BLOCK();
		bb_graphics_EndRender();
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<115>");
	m__graphics->EndRender();
}
void c_GameDelegate::KeyEvent(int t_event,int t_data){
	DBG_ENTER("GameDelegate.KeyEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<119>");
	m__input->p_KeyEvent(t_event,t_data);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<120>");
	if(t_event!=1){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<121>");
	int t_1=t_data;
	DBG_LOCAL(t_1,"1")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<122>");
	if(t_1==432){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<123>");
		bb_app__app->p_OnClose();
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<124>");
		if(t_1==416){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<125>");
			bb_app__app->p_OnBack();
		}
	}
}
void c_GameDelegate::MouseEvent(int t_event,int t_data,Float t_x,Float t_y){
	DBG_ENTER("GameDelegate.MouseEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<130>");
	m__input->p_MouseEvent(t_event,t_data,t_x,t_y);
}
void c_GameDelegate::TouchEvent(int t_event,int t_data,Float t_x,Float t_y){
	DBG_ENTER("GameDelegate.TouchEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<134>");
	m__input->p_TouchEvent(t_event,t_data,t_x,t_y);
}
void c_GameDelegate::MotionEvent(int t_event,int t_data,Float t_x,Float t_y,Float t_z){
	DBG_ENTER("GameDelegate.MotionEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_z,"z")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<138>");
	m__input->p_MotionEvent(t_event,t_data,t_x,t_y,t_z);
}
void c_GameDelegate::DiscardGraphics(){
	DBG_ENTER("GameDelegate.DiscardGraphics")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<142>");
	m__graphics->DiscardGraphics();
}
void c_GameDelegate::mark(){
	BBGameDelegate::mark();
	gc_mark_q(m__graphics);
	gc_mark_q(m__audio);
	gc_mark_q(m__input);
}
String c_GameDelegate::debug(){
	String t="(GameDelegate)\n";
	t+=dbg_decl("_graphics",&m__graphics);
	t+=dbg_decl("_audio",&m__audio);
	t+=dbg_decl("_input",&m__input);
	return t;
}
c_GameDelegate* bb_app__delegate;
BBGame* bb_app__game;
int bbMain(){
	DBG_ENTER("Main")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<83>");
	c_Game* t_g=(new c_Game)->m_new();
	DBG_LOCAL(t_g,"g")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<84>");
	t_g->p_DrawSomewhere();
	return 0;
}
gxtkGraphics* bb_graphics_device;
int bb_graphics_SetGraphicsDevice(gxtkGraphics* t_dev){
	DBG_ENTER("SetGraphicsDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<63>");
	gc_assign(bb_graphics_device,t_dev);
	return 0;
}
c_Image::c_Image(){
	m_surface=0;
	m_width=0;
	m_height=0;
	m_frames=Array<c_Frame* >();
	m_flags=0;
	m_tx=FLOAT(.0);
	m_ty=FLOAT(.0);
	m_source=0;
}
int c_Image::m_DefaultFlags;
c_Image* c_Image::m_new(){
	DBG_ENTER("Image.new")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<70>");
	return this;
}
int c_Image::p_SetHandle(Float t_tx,Float t_ty){
	DBG_ENTER("Image.SetHandle")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<114>");
	this->m_tx=t_tx;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<115>");
	this->m_ty=t_ty;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<116>");
	this->m_flags=this->m_flags&-2;
	return 0;
}
int c_Image::p_ApplyFlags(int t_iflags){
	DBG_ENTER("Image.ApplyFlags")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<197>");
	m_flags=t_iflags;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<199>");
	if((m_flags&2)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<200>");
		Array<c_Frame* > t_=m_frames;
		int t_2=0;
		while(t_2<t_.Length()){
			DBG_BLOCK();
			c_Frame* t_f=t_.At(t_2);
			t_2=t_2+1;
			DBG_LOCAL(t_f,"f")
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<201>");
			t_f->m_x+=1;
		}
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<203>");
		m_width-=2;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<206>");
	if((m_flags&4)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<207>");
		Array<c_Frame* > t_3=m_frames;
		int t_4=0;
		while(t_4<t_3.Length()){
			DBG_BLOCK();
			c_Frame* t_f2=t_3.At(t_4);
			t_4=t_4+1;
			DBG_LOCAL(t_f2,"f")
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<208>");
			t_f2->m_y+=1;
		}
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<210>");
		m_height-=2;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<213>");
	if((m_flags&1)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<214>");
		p_SetHandle(Float(m_width)/FLOAT(2.0),Float(m_height)/FLOAT(2.0));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<217>");
	if(m_frames.Length()==1 && m_frames.At(0)->m_x==0 && m_frames.At(0)->m_y==0 && m_width==m_surface->Width() && m_height==m_surface->Height()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<218>");
		m_flags|=65536;
	}
	return 0;
}
c_Image* c_Image::p_Init(gxtkSurface* t_surf,int t_nframes,int t_iflags){
	DBG_ENTER("Image.Init")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_surf,"surf")
	DBG_LOCAL(t_nframes,"nframes")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<143>");
	if((m_surface)!=0){
		DBG_BLOCK();
		bbError(String(L"Image already initialized",25));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<144>");
	gc_assign(m_surface,t_surf);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<146>");
	m_width=m_surface->Width()/t_nframes;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<147>");
	m_height=m_surface->Height();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<149>");
	gc_assign(m_frames,Array<c_Frame* >(t_nframes));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<150>");
	for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<151>");
		gc_assign(m_frames.At(t_i),(new c_Frame)->m_new(t_i*m_width,0));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<154>");
	p_ApplyFlags(t_iflags);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<155>");
	return this;
}
c_Image* c_Image::p_Init2(gxtkSurface* t_surf,int t_x,int t_y,int t_iwidth,int t_iheight,int t_nframes,int t_iflags,c_Image* t_src,int t_srcx,int t_srcy,int t_srcw,int t_srch){
	DBG_ENTER("Image.Init")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_surf,"surf")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_iwidth,"iwidth")
	DBG_LOCAL(t_iheight,"iheight")
	DBG_LOCAL(t_nframes,"nframes")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_LOCAL(t_src,"src")
	DBG_LOCAL(t_srcx,"srcx")
	DBG_LOCAL(t_srcy,"srcy")
	DBG_LOCAL(t_srcw,"srcw")
	DBG_LOCAL(t_srch,"srch")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<159>");
	if((m_surface)!=0){
		DBG_BLOCK();
		bbError(String(L"Image already initialized",25));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<160>");
	gc_assign(m_surface,t_surf);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<161>");
	gc_assign(m_source,t_src);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<163>");
	m_width=t_iwidth;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<164>");
	m_height=t_iheight;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<166>");
	gc_assign(m_frames,Array<c_Frame* >(t_nframes));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<168>");
	int t_ix=t_x;
	int t_iy=t_y;
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<170>");
	for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<171>");
		if(t_ix+m_width>t_srcw){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<172>");
			t_ix=0;
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<173>");
			t_iy+=m_height;
		}
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<175>");
		if(t_ix+m_width>t_srcw || t_iy+m_height>t_srch){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<176>");
			bbError(String(L"Image frame outside surface",27));
		}
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<178>");
		gc_assign(m_frames.At(t_i),(new c_Frame)->m_new(t_ix+t_srcx,t_iy+t_srcy));
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<179>");
		t_ix+=m_width;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<182>");
	p_ApplyFlags(t_iflags);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<183>");
	return this;
}
void c_Image::mark(){
	Object::mark();
	gc_mark_q(m_surface);
	gc_mark_q(m_frames);
	gc_mark_q(m_source);
}
String c_Image::debug(){
	String t="(Image)\n";
	t+=dbg_decl("DefaultFlags",&c_Image::m_DefaultFlags);
	t+=dbg_decl("source",&m_source);
	t+=dbg_decl("surface",&m_surface);
	t+=dbg_decl("width",&m_width);
	t+=dbg_decl("height",&m_height);
	t+=dbg_decl("flags",&m_flags);
	t+=dbg_decl("frames",&m_frames);
	t+=dbg_decl("tx",&m_tx);
	t+=dbg_decl("ty",&m_ty);
	return t;
}
c_GraphicsContext::c_GraphicsContext(){
	m_defaultFont=0;
	m_font=0;
	m_firstChar=0;
	m_matrixSp=0;
	m_ix=FLOAT(1.0);
	m_iy=FLOAT(.0);
	m_jx=FLOAT(.0);
	m_jy=FLOAT(1.0);
	m_tx=FLOAT(.0);
	m_ty=FLOAT(.0);
	m_tformed=0;
	m_matDirty=0;
	m_color_r=FLOAT(.0);
	m_color_g=FLOAT(.0);
	m_color_b=FLOAT(.0);
	m_alpha=FLOAT(.0);
	m_blend=0;
	m_scissor_x=FLOAT(.0);
	m_scissor_y=FLOAT(.0);
	m_scissor_width=FLOAT(.0);
	m_scissor_height=FLOAT(.0);
	m_matrixStack=Array<Float >(192);
}
c_GraphicsContext* c_GraphicsContext::m_new(){
	DBG_ENTER("GraphicsContext.new")
	c_GraphicsContext *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<29>");
	return this;
}
int c_GraphicsContext::p_Validate(){
	DBG_ENTER("GraphicsContext.Validate")
	c_GraphicsContext *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<40>");
	if((m_matDirty)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<41>");
		bb_graphics_renderDevice->SetMatrix(bb_graphics_context->m_ix,bb_graphics_context->m_iy,bb_graphics_context->m_jx,bb_graphics_context->m_jy,bb_graphics_context->m_tx,bb_graphics_context->m_ty);
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<42>");
		m_matDirty=0;
	}
	return 0;
}
void c_GraphicsContext::mark(){
	Object::mark();
	gc_mark_q(m_defaultFont);
	gc_mark_q(m_font);
	gc_mark_q(m_matrixStack);
}
String c_GraphicsContext::debug(){
	String t="(GraphicsContext)\n";
	t+=dbg_decl("color_r",&m_color_r);
	t+=dbg_decl("color_g",&m_color_g);
	t+=dbg_decl("color_b",&m_color_b);
	t+=dbg_decl("alpha",&m_alpha);
	t+=dbg_decl("blend",&m_blend);
	t+=dbg_decl("ix",&m_ix);
	t+=dbg_decl("iy",&m_iy);
	t+=dbg_decl("jx",&m_jx);
	t+=dbg_decl("jy",&m_jy);
	t+=dbg_decl("tx",&m_tx);
	t+=dbg_decl("ty",&m_ty);
	t+=dbg_decl("tformed",&m_tformed);
	t+=dbg_decl("matDirty",&m_matDirty);
	t+=dbg_decl("scissor_x",&m_scissor_x);
	t+=dbg_decl("scissor_y",&m_scissor_y);
	t+=dbg_decl("scissor_width",&m_scissor_width);
	t+=dbg_decl("scissor_height",&m_scissor_height);
	t+=dbg_decl("matrixStack",&m_matrixStack);
	t+=dbg_decl("matrixSp",&m_matrixSp);
	t+=dbg_decl("font",&m_font);
	t+=dbg_decl("firstChar",&m_firstChar);
	t+=dbg_decl("defaultFont",&m_defaultFont);
	return t;
}
c_GraphicsContext* bb_graphics_context;
String bb_data_FixDataPath(String t_path){
	DBG_ENTER("FixDataPath")
	DBG_LOCAL(t_path,"path")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/data.monkey<7>");
	int t_i=t_path.Find(String(L":/",2),0);
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/data.monkey<8>");
	if(t_i!=-1 && t_path.Find(String(L"/",1),0)==t_i+1){
		DBG_BLOCK();
		return t_path;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/data.monkey<9>");
	if(t_path.StartsWith(String(L"./",2)) || t_path.StartsWith(String(L"/",1))){
		DBG_BLOCK();
		return t_path;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/data.monkey<10>");
	String t_=String(L"monkey://data/",14)+t_path;
	return t_;
}
c_Frame::c_Frame(){
	m_x=0;
	m_y=0;
}
c_Frame* c_Frame::m_new(int t_x,int t_y){
	DBG_ENTER("Frame.new")
	c_Frame *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<23>");
	this->m_x=t_x;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<24>");
	this->m_y=t_y;
	return this;
}
c_Frame* c_Frame::m_new2(){
	DBG_ENTER("Frame.new")
	c_Frame *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<18>");
	return this;
}
void c_Frame::mark(){
	Object::mark();
}
String c_Frame::debug(){
	String t="(Frame)\n";
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	return t;
}
c_Image* bb_graphics_LoadImage(String t_path,int t_frameCount,int t_flags){
	DBG_ENTER("LoadImage")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<239>");
	gxtkSurface* t_surf=bb_graphics_device->LoadSurface(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_surf,"surf")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<240>");
	if((t_surf)!=0){
		DBG_BLOCK();
		c_Image* t_=((new c_Image)->m_new())->p_Init(t_surf,t_frameCount,t_flags);
		return t_;
	}
	return 0;
}
c_Image* bb_graphics_LoadImage2(String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount,int t_flags){
	DBG_ENTER("LoadImage")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<244>");
	gxtkSurface* t_surf=bb_graphics_device->LoadSurface(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_surf,"surf")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<245>");
	if((t_surf)!=0){
		DBG_BLOCK();
		c_Image* t_=((new c_Image)->m_new())->p_Init2(t_surf,0,0,t_frameWidth,t_frameHeight,t_frameCount,t_flags,0,0,0,t_surf->Width(),t_surf->Height());
		return t_;
	}
	return 0;
}
int bb_graphics_SetFont(c_Image* t_font,int t_firstChar){
	DBG_ENTER("SetFont")
	DBG_LOCAL(t_font,"font")
	DBG_LOCAL(t_firstChar,"firstChar")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<548>");
	if(!((t_font)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<549>");
		if(!((bb_graphics_context->m_defaultFont)!=0)){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<550>");
			gc_assign(bb_graphics_context->m_defaultFont,bb_graphics_LoadImage(String(L"mojo_font.png",13),96,2));
		}
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<552>");
		t_font=bb_graphics_context->m_defaultFont;
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<553>");
		t_firstChar=32;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<555>");
	gc_assign(bb_graphics_context->m_font,t_font);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<556>");
	bb_graphics_context->m_firstChar=t_firstChar;
	return 0;
}
gxtkAudio* bb_audio_device;
int bb_audio_SetAudioDevice(gxtkAudio* t_dev){
	DBG_ENTER("SetAudioDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/audio.monkey<22>");
	gc_assign(bb_audio_device,t_dev);
	return 0;
}
c_InputDevice::c_InputDevice(){
	m__joyStates=Array<c_JoyState* >(4);
	m__keyDown=Array<bool >(512);
	m__keyHitPut=0;
	m__keyHitQueue=Array<int >(33);
	m__keyHit=Array<int >(512);
	m__charGet=0;
	m__charPut=0;
	m__charQueue=Array<int >(32);
	m__mouseX=FLOAT(.0);
	m__mouseY=FLOAT(.0);
	m__touchX=Array<Float >(32);
	m__touchY=Array<Float >(32);
	m__accelX=FLOAT(.0);
	m__accelY=FLOAT(.0);
	m__accelZ=FLOAT(.0);
}
c_InputDevice* c_InputDevice::m_new(){
	DBG_ENTER("InputDevice.new")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<26>");
	for(int t_i=0;t_i<4;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<27>");
		gc_assign(m__joyStates.At(t_i),(new c_JoyState)->m_new());
	}
	return this;
}
void c_InputDevice::p_PutKeyHit(int t_key){
	DBG_ENTER("InputDevice.PutKeyHit")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<237>");
	if(m__keyHitPut==m__keyHitQueue.Length()){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<238>");
	m__keyHit.At(t_key)+=1;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<239>");
	m__keyHitQueue.At(m__keyHitPut)=t_key;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<240>");
	m__keyHitPut+=1;
}
void c_InputDevice::p_BeginUpdate(){
	DBG_ENTER("InputDevice.BeginUpdate")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<189>");
	for(int t_i=0;t_i<4;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<190>");
		c_JoyState* t_state=m__joyStates.At(t_i);
		DBG_LOCAL(t_state,"state")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<191>");
		if(!BBGame::Game()->PollJoystick(t_i,t_state->m_joyx,t_state->m_joyy,t_state->m_joyz,t_state->m_buttons)){
			DBG_BLOCK();
			break;
		}
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<192>");
		for(int t_j=0;t_j<32;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<193>");
			int t_key=256+t_i*32+t_j;
			DBG_LOCAL(t_key,"key")
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<194>");
			if(t_state->m_buttons.At(t_j)){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<195>");
				if(!m__keyDown.At(t_key)){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<196>");
					m__keyDown.At(t_key)=true;
					DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<197>");
					p_PutKeyHit(t_key);
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<200>");
				m__keyDown.At(t_key)=false;
			}
		}
	}
}
void c_InputDevice::p_EndUpdate(){
	DBG_ENTER("InputDevice.EndUpdate")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<207>");
	for(int t_i=0;t_i<m__keyHitPut;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<208>");
		m__keyHit.At(m__keyHitQueue.At(t_i))=0;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<210>");
	m__keyHitPut=0;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<211>");
	m__charGet=0;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<212>");
	m__charPut=0;
}
void c_InputDevice::p_KeyEvent(int t_event,int t_data){
	DBG_ENTER("InputDevice.KeyEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<111>");
	int t_1=t_event;
	DBG_LOCAL(t_1,"1")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<112>");
	if(t_1==1){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<113>");
		if(!m__keyDown.At(t_data)){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<114>");
			m__keyDown.At(t_data)=true;
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<115>");
			p_PutKeyHit(t_data);
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<116>");
			if(t_data==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<117>");
				m__keyDown.At(384)=true;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<118>");
				p_PutKeyHit(384);
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<119>");
				if(t_data==384){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<120>");
					m__keyDown.At(1)=true;
					DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<121>");
					p_PutKeyHit(1);
				}
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<124>");
		if(t_1==2){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<125>");
			if(m__keyDown.At(t_data)){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<126>");
				m__keyDown.At(t_data)=false;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<127>");
				if(t_data==1){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<128>");
					m__keyDown.At(384)=false;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<129>");
					if(t_data==384){
						DBG_BLOCK();
						DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<130>");
						m__keyDown.At(1)=false;
					}
				}
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<133>");
			if(t_1==3){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<134>");
				if(m__charPut<m__charQueue.Length()){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<135>");
					m__charQueue.At(m__charPut)=t_data;
					DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<136>");
					m__charPut+=1;
				}
			}
		}
	}
}
void c_InputDevice::p_MouseEvent(int t_event,int t_data,Float t_x,Float t_y){
	DBG_ENTER("InputDevice.MouseEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<142>");
	int t_2=t_event;
	DBG_LOCAL(t_2,"2")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<143>");
	if(t_2==4){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<144>");
		p_KeyEvent(1,1+t_data);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<145>");
		if(t_2==5){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<146>");
			p_KeyEvent(2,1+t_data);
			return;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<148>");
			if(t_2==6){
				DBG_BLOCK();
			}else{
				DBG_BLOCK();
				return;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<152>");
	m__mouseX=t_x;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<153>");
	m__mouseY=t_y;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<154>");
	m__touchX.At(0)=t_x;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<155>");
	m__touchY.At(0)=t_y;
}
void c_InputDevice::p_TouchEvent(int t_event,int t_data,Float t_x,Float t_y){
	DBG_ENTER("InputDevice.TouchEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<159>");
	int t_3=t_event;
	DBG_LOCAL(t_3,"3")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<160>");
	if(t_3==7){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<161>");
		p_KeyEvent(1,384+t_data);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<162>");
		if(t_3==8){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<163>");
			p_KeyEvent(2,384+t_data);
			return;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<165>");
			if(t_3==9){
				DBG_BLOCK();
			}else{
				DBG_BLOCK();
				return;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<169>");
	m__touchX.At(t_data)=t_x;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<170>");
	m__touchY.At(t_data)=t_y;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<171>");
	if(t_data==0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<172>");
		m__mouseX=t_x;
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<173>");
		m__mouseY=t_y;
	}
}
void c_InputDevice::p_MotionEvent(int t_event,int t_data,Float t_x,Float t_y,Float t_z){
	DBG_ENTER("InputDevice.MotionEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_z,"z")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<178>");
	int t_4=t_event;
	DBG_LOCAL(t_4,"4")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<179>");
	if(t_4==10){
		DBG_BLOCK();
	}else{
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<183>");
	m__accelX=t_x;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<184>");
	m__accelY=t_y;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<185>");
	m__accelZ=t_z;
}
bool c_InputDevice::p_KeyDown(int t_key){
	DBG_ENTER("InputDevice.KeyDown")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<47>");
	if(t_key>0 && t_key<512){
		DBG_BLOCK();
		return m__keyDown.At(t_key);
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<48>");
	return false;
}
void c_InputDevice::mark(){
	Object::mark();
	gc_mark_q(m__joyStates);
	gc_mark_q(m__keyDown);
	gc_mark_q(m__keyHitQueue);
	gc_mark_q(m__keyHit);
	gc_mark_q(m__charQueue);
	gc_mark_q(m__touchX);
	gc_mark_q(m__touchY);
}
String c_InputDevice::debug(){
	String t="(InputDevice)\n";
	t+=dbg_decl("_keyDown",&m__keyDown);
	t+=dbg_decl("_keyHit",&m__keyHit);
	t+=dbg_decl("_keyHitQueue",&m__keyHitQueue);
	t+=dbg_decl("_keyHitPut",&m__keyHitPut);
	t+=dbg_decl("_charQueue",&m__charQueue);
	t+=dbg_decl("_charPut",&m__charPut);
	t+=dbg_decl("_charGet",&m__charGet);
	t+=dbg_decl("_mouseX",&m__mouseX);
	t+=dbg_decl("_mouseY",&m__mouseY);
	t+=dbg_decl("_touchX",&m__touchX);
	t+=dbg_decl("_touchY",&m__touchY);
	t+=dbg_decl("_accelX",&m__accelX);
	t+=dbg_decl("_accelY",&m__accelY);
	t+=dbg_decl("_accelZ",&m__accelZ);
	t+=dbg_decl("_joyStates",&m__joyStates);
	return t;
}
c_JoyState::c_JoyState(){
	m_joyx=Array<Float >(2);
	m_joyy=Array<Float >(2);
	m_joyz=Array<Float >(2);
	m_buttons=Array<bool >(32);
}
c_JoyState* c_JoyState::m_new(){
	DBG_ENTER("JoyState.new")
	c_JoyState *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/inputdevice.monkey<14>");
	return this;
}
void c_JoyState::mark(){
	Object::mark();
	gc_mark_q(m_joyx);
	gc_mark_q(m_joyy);
	gc_mark_q(m_joyz);
	gc_mark_q(m_buttons);
}
String c_JoyState::debug(){
	String t="(JoyState)\n";
	t+=dbg_decl("joyx",&m_joyx);
	t+=dbg_decl("joyy",&m_joyy);
	t+=dbg_decl("joyz",&m_joyz);
	t+=dbg_decl("buttons",&m_buttons);
	return t;
}
c_InputDevice* bb_input_device;
int bb_input_SetInputDevice(c_InputDevice* t_dev){
	DBG_ENTER("SetInputDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/input.monkey<22>");
	gc_assign(bb_input_device,t_dev);
	return 0;
}
int bb_app__devWidth;
int bb_app__devHeight;
void bb_app_ValidateDeviceWindow(bool t_notifyApp){
	DBG_ENTER("ValidateDeviceWindow")
	DBG_LOCAL(t_notifyApp,"notifyApp")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<57>");
	int t_w=bb_app__game->GetDeviceWidth();
	DBG_LOCAL(t_w,"w")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<58>");
	int t_h=bb_app__game->GetDeviceHeight();
	DBG_LOCAL(t_h,"h")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<59>");
	if(t_w==bb_app__devWidth && t_h==bb_app__devHeight){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<60>");
	bb_app__devWidth=t_w;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<61>");
	bb_app__devHeight=t_h;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<62>");
	if(t_notifyApp){
		DBG_BLOCK();
		bb_app__app->p_OnResize();
	}
}
c_DisplayMode::c_DisplayMode(){
	m__width=0;
	m__height=0;
}
c_DisplayMode* c_DisplayMode::m_new(int t_width,int t_height){
	DBG_ENTER("DisplayMode.new")
	c_DisplayMode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<192>");
	m__width=t_width;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<193>");
	m__height=t_height;
	return this;
}
c_DisplayMode* c_DisplayMode::m_new2(){
	DBG_ENTER("DisplayMode.new")
	c_DisplayMode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<189>");
	return this;
}
void c_DisplayMode::mark(){
	Object::mark();
}
String c_DisplayMode::debug(){
	String t="(DisplayMode)\n";
	t+=dbg_decl("_width",&m__width);
	t+=dbg_decl("_height",&m__height);
	return t;
}
c_Map::c_Map(){
	m_root=0;
}
c_Map* c_Map::m_new(){
	DBG_ENTER("Map.new")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<7>");
	return this;
}
c_Node* c_Map::p_FindNode(int t_key){
	DBG_ENTER("Map.FindNode")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<157>");
	c_Node* t_node=m_root;
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<159>");
	while((t_node)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<160>");
		int t_cmp=p_Compare(t_key,t_node->m_key);
		DBG_LOCAL(t_cmp,"cmp")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<161>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<162>");
			t_node=t_node->m_right;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<163>");
			if(t_cmp<0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<164>");
				t_node=t_node->m_left;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<166>");
				return t_node;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<169>");
	return t_node;
}
bool c_Map::p_Contains(int t_key){
	DBG_ENTER("Map.Contains")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<25>");
	bool t_=p_FindNode(t_key)!=0;
	return t_;
}
int c_Map::p_RotateLeft(c_Node* t_node){
	DBG_ENTER("Map.RotateLeft")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<251>");
	c_Node* t_child=t_node->m_right;
	DBG_LOCAL(t_child,"child")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<252>");
	gc_assign(t_node->m_right,t_child->m_left);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<253>");
	if((t_child->m_left)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<254>");
		gc_assign(t_child->m_left->m_parent,t_node);
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<256>");
	gc_assign(t_child->m_parent,t_node->m_parent);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<257>");
	if((t_node->m_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<258>");
		if(t_node==t_node->m_parent->m_left){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<259>");
			gc_assign(t_node->m_parent->m_left,t_child);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<261>");
			gc_assign(t_node->m_parent->m_right,t_child);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<264>");
		gc_assign(m_root,t_child);
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<266>");
	gc_assign(t_child->m_left,t_node);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<267>");
	gc_assign(t_node->m_parent,t_child);
	return 0;
}
int c_Map::p_RotateRight(c_Node* t_node){
	DBG_ENTER("Map.RotateRight")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<271>");
	c_Node* t_child=t_node->m_left;
	DBG_LOCAL(t_child,"child")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<272>");
	gc_assign(t_node->m_left,t_child->m_right);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<273>");
	if((t_child->m_right)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<274>");
		gc_assign(t_child->m_right->m_parent,t_node);
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<276>");
	gc_assign(t_child->m_parent,t_node->m_parent);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<277>");
	if((t_node->m_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<278>");
		if(t_node==t_node->m_parent->m_right){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<279>");
			gc_assign(t_node->m_parent->m_right,t_child);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<281>");
			gc_assign(t_node->m_parent->m_left,t_child);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<284>");
		gc_assign(m_root,t_child);
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<286>");
	gc_assign(t_child->m_right,t_node);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<287>");
	gc_assign(t_node->m_parent,t_child);
	return 0;
}
int c_Map::p_InsertFixup(c_Node* t_node){
	DBG_ENTER("Map.InsertFixup")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<212>");
	while(((t_node->m_parent)!=0) && t_node->m_parent->m_color==-1 && ((t_node->m_parent->m_parent)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<213>");
		if(t_node->m_parent==t_node->m_parent->m_parent->m_left){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<214>");
			c_Node* t_uncle=t_node->m_parent->m_parent->m_right;
			DBG_LOCAL(t_uncle,"uncle")
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<215>");
			if(((t_uncle)!=0) && t_uncle->m_color==-1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<216>");
				t_node->m_parent->m_color=1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<217>");
				t_uncle->m_color=1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<218>");
				t_uncle->m_parent->m_color=-1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<219>");
				t_node=t_uncle->m_parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<221>");
				if(t_node==t_node->m_parent->m_right){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<222>");
					t_node=t_node->m_parent;
					DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<223>");
					p_RotateLeft(t_node);
				}
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<225>");
				t_node->m_parent->m_color=1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<226>");
				t_node->m_parent->m_parent->m_color=-1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<227>");
				p_RotateRight(t_node->m_parent->m_parent);
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<230>");
			c_Node* t_uncle2=t_node->m_parent->m_parent->m_left;
			DBG_LOCAL(t_uncle2,"uncle")
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<231>");
			if(((t_uncle2)!=0) && t_uncle2->m_color==-1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<232>");
				t_node->m_parent->m_color=1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<233>");
				t_uncle2->m_color=1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<234>");
				t_uncle2->m_parent->m_color=-1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<235>");
				t_node=t_uncle2->m_parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<237>");
				if(t_node==t_node->m_parent->m_left){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<238>");
					t_node=t_node->m_parent;
					DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<239>");
					p_RotateRight(t_node);
				}
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<241>");
				t_node->m_parent->m_color=1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<242>");
				t_node->m_parent->m_parent->m_color=-1;
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<243>");
				p_RotateLeft(t_node->m_parent->m_parent);
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<247>");
	m_root->m_color=1;
	return 0;
}
bool c_Map::p_Set(int t_key,c_DisplayMode* t_value){
	DBG_ENTER("Map.Set")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<29>");
	c_Node* t_node=m_root;
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<30>");
	c_Node* t_parent=0;
	int t_cmp=0;
	DBG_LOCAL(t_parent,"parent")
	DBG_LOCAL(t_cmp,"cmp")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<32>");
	while((t_node)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<33>");
		t_parent=t_node;
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<34>");
		t_cmp=p_Compare(t_key,t_node->m_key);
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<35>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<36>");
			t_node=t_node->m_right;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<37>");
			if(t_cmp<0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<38>");
				t_node=t_node->m_left;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<40>");
				gc_assign(t_node->m_value,t_value);
				DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<41>");
				return false;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<45>");
	t_node=(new c_Node)->m_new(t_key,t_value,-1,t_parent);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<47>");
	if((t_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<48>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<49>");
			gc_assign(t_parent->m_right,t_node);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<51>");
			gc_assign(t_parent->m_left,t_node);
		}
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<53>");
		p_InsertFixup(t_node);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<55>");
		gc_assign(m_root,t_node);
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<57>");
	return true;
}
bool c_Map::p_Insert(int t_key,c_DisplayMode* t_value){
	DBG_ENTER("Map.Insert")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<146>");
	bool t_=p_Set(t_key,t_value);
	return t_;
}
void c_Map::mark(){
	Object::mark();
	gc_mark_q(m_root);
}
String c_Map::debug(){
	String t="(Map)\n";
	t+=dbg_decl("root",&m_root);
	return t;
}
c_IntMap::c_IntMap(){
}
c_IntMap* c_IntMap::m_new(){
	DBG_ENTER("IntMap.new")
	c_IntMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<534>");
	c_Map::m_new();
	return this;
}
int c_IntMap::p_Compare(int t_lhs,int t_rhs){
	DBG_ENTER("IntMap.Compare")
	c_IntMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_lhs,"lhs")
	DBG_LOCAL(t_rhs,"rhs")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<537>");
	int t_=t_lhs-t_rhs;
	return t_;
}
void c_IntMap::mark(){
	c_Map::mark();
}
String c_IntMap::debug(){
	String t="(IntMap)\n";
	t=c_Map::debug()+t;
	return t;
}
c_Stack::c_Stack(){
	m_data=Array<c_DisplayMode* >();
	m_length=0;
}
c_Stack* c_Stack::m_new(){
	DBG_ENTER("Stack.new")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
c_Stack* c_Stack::m_new2(Array<c_DisplayMode* > t_data){
	DBG_ENTER("Stack.new")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<13>");
	gc_assign(this->m_data,t_data.Slice(0));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<14>");
	this->m_length=t_data.Length();
	return this;
}
void c_Stack::p_Push(c_DisplayMode* t_value){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<71>");
	if(m_length==m_data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<72>");
		gc_assign(m_data,m_data.Resize(m_length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<74>");
	gc_assign(m_data.At(m_length),t_value);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<75>");
	m_length+=1;
}
void c_Stack::p_Push2(Array<c_DisplayMode* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<83>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<84>");
		p_Push(t_values.At(t_offset+t_i));
	}
}
void c_Stack::p_Push3(Array<c_DisplayMode* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<79>");
	p_Push2(t_values,t_offset,t_values.Length()-t_offset);
}
Array<c_DisplayMode* > c_Stack::p_ToArray(){
	DBG_ENTER("Stack.ToArray")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<18>");
	Array<c_DisplayMode* > t_t=Array<c_DisplayMode* >(m_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<m_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<20>");
		gc_assign(t_t.At(t_i),m_data.At(t_i));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/stack.monkey<22>");
	return t_t;
}
void c_Stack::mark(){
	Object::mark();
	gc_mark_q(m_data);
}
String c_Stack::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("data",&m_data);
	t+=dbg_decl("length",&m_length);
	return t;
}
c_Node::c_Node(){
	m_key=0;
	m_right=0;
	m_left=0;
	m_value=0;
	m_color=0;
	m_parent=0;
}
c_Node* c_Node::m_new(int t_key,c_DisplayMode* t_value,int t_color,c_Node* t_parent){
	DBG_ENTER("Node.new")
	c_Node *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_LOCAL(t_color,"color")
	DBG_LOCAL(t_parent,"parent")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<364>");
	this->m_key=t_key;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<365>");
	gc_assign(this->m_value,t_value);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<366>");
	this->m_color=t_color;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<367>");
	gc_assign(this->m_parent,t_parent);
	return this;
}
c_Node* c_Node::m_new2(){
	DBG_ENTER("Node.new")
	c_Node *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/monkey/map.monkey<361>");
	return this;
}
void c_Node::mark(){
	Object::mark();
	gc_mark_q(m_right);
	gc_mark_q(m_left);
	gc_mark_q(m_value);
	gc_mark_q(m_parent);
}
String c_Node::debug(){
	String t="(Node)\n";
	t+=dbg_decl("key",&m_key);
	t+=dbg_decl("value",&m_value);
	t+=dbg_decl("color",&m_color);
	t+=dbg_decl("parent",&m_parent);
	t+=dbg_decl("left",&m_left);
	t+=dbg_decl("right",&m_right);
	return t;
}
Array<c_DisplayMode* > bb_app__displayModes;
c_DisplayMode* bb_app__desktopMode;
int bb_app_DeviceWidth(){
	DBG_ENTER("DeviceWidth")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<263>");
	return bb_app__devWidth;
}
int bb_app_DeviceHeight(){
	DBG_ENTER("DeviceHeight")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<267>");
	return bb_app__devHeight;
}
void bb_app_EnumDisplayModes(){
	DBG_ENTER("EnumDisplayModes")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<33>");
	Array<BBDisplayMode* > t_modes=bb_app__game->GetDisplayModes();
	DBG_LOCAL(t_modes,"modes")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<34>");
	c_IntMap* t_mmap=(new c_IntMap)->m_new();
	DBG_LOCAL(t_mmap,"mmap")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<35>");
	c_Stack* t_mstack=(new c_Stack)->m_new();
	DBG_LOCAL(t_mstack,"mstack")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<36>");
	for(int t_i=0;t_i<t_modes.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<37>");
		int t_w=t_modes.At(t_i)->width;
		DBG_LOCAL(t_w,"w")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<38>");
		int t_h=t_modes.At(t_i)->height;
		DBG_LOCAL(t_h,"h")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<39>");
		int t_size=t_w<<16|t_h;
		DBG_LOCAL(t_size,"size")
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<40>");
		if(t_mmap->p_Contains(t_size)){
			DBG_BLOCK();
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<42>");
			c_DisplayMode* t_mode=(new c_DisplayMode)->m_new(t_modes.At(t_i)->width,t_modes.At(t_i)->height);
			DBG_LOCAL(t_mode,"mode")
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<43>");
			t_mmap->p_Insert(t_size,t_mode);
			DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<44>");
			t_mstack->p_Push(t_mode);
		}
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<47>");
	gc_assign(bb_app__displayModes,t_mstack->p_ToArray());
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<48>");
	BBDisplayMode* t_mode2=bb_app__game->GetDesktopMode();
	DBG_LOCAL(t_mode2,"mode")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<49>");
	if((t_mode2)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<50>");
		gc_assign(bb_app__desktopMode,(new c_DisplayMode)->m_new(t_mode2->width,t_mode2->height));
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<52>");
		gc_assign(bb_app__desktopMode,(new c_DisplayMode)->m_new(bb_app_DeviceWidth(),bb_app_DeviceHeight()));
	}
}
gxtkGraphics* bb_graphics_renderDevice;
int bb_graphics_SetMatrix(Float t_ix,Float t_iy,Float t_jx,Float t_jy,Float t_tx,Float t_ty){
	DBG_ENTER("SetMatrix")
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_LOCAL(t_jx,"jx")
	DBG_LOCAL(t_jy,"jy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<312>");
	bb_graphics_context->m_ix=t_ix;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<313>");
	bb_graphics_context->m_iy=t_iy;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<314>");
	bb_graphics_context->m_jx=t_jx;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<315>");
	bb_graphics_context->m_jy=t_jy;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<316>");
	bb_graphics_context->m_tx=t_tx;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<317>");
	bb_graphics_context->m_ty=t_ty;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<318>");
	bb_graphics_context->m_tformed=((t_ix!=FLOAT(1.0) || t_iy!=FLOAT(0.0) || t_jx!=FLOAT(0.0) || t_jy!=FLOAT(1.0) || t_tx!=FLOAT(0.0) || t_ty!=FLOAT(0.0))?1:0);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<319>");
	bb_graphics_context->m_matDirty=1;
	return 0;
}
int bb_graphics_SetMatrix2(Array<Float > t_m){
	DBG_ENTER("SetMatrix")
	DBG_LOCAL(t_m,"m")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<308>");
	bb_graphics_SetMatrix(t_m.At(0),t_m.At(1),t_m.At(2),t_m.At(3),t_m.At(4),t_m.At(5));
	return 0;
}
int bb_graphics_SetColor(Float t_r,Float t_g,Float t_b){
	DBG_ENTER("SetColor")
	DBG_LOCAL(t_r,"r")
	DBG_LOCAL(t_g,"g")
	DBG_LOCAL(t_b,"b")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<254>");
	bb_graphics_context->m_color_r=t_r;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<255>");
	bb_graphics_context->m_color_g=t_g;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<256>");
	bb_graphics_context->m_color_b=t_b;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<257>");
	bb_graphics_renderDevice->SetColor(t_r,t_g,t_b);
	return 0;
}
int bb_graphics_SetAlpha(Float t_alpha){
	DBG_ENTER("SetAlpha")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<271>");
	bb_graphics_context->m_alpha=t_alpha;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<272>");
	bb_graphics_renderDevice->SetAlpha(t_alpha);
	return 0;
}
int bb_graphics_SetBlend(int t_blend){
	DBG_ENTER("SetBlend")
	DBG_LOCAL(t_blend,"blend")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<280>");
	bb_graphics_context->m_blend=t_blend;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<281>");
	bb_graphics_renderDevice->SetBlend(t_blend);
	return 0;
}
int bb_graphics_SetScissor(Float t_x,Float t_y,Float t_width,Float t_height){
	DBG_ENTER("SetScissor")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<289>");
	bb_graphics_context->m_scissor_x=t_x;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<290>");
	bb_graphics_context->m_scissor_y=t_y;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<291>");
	bb_graphics_context->m_scissor_width=t_width;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<292>");
	bb_graphics_context->m_scissor_height=t_height;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<293>");
	bb_graphics_renderDevice->SetScissor(int(t_x),int(t_y),int(t_width),int(t_height));
	return 0;
}
int bb_graphics_BeginRender(){
	DBG_ENTER("BeginRender")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<225>");
	gc_assign(bb_graphics_renderDevice,bb_graphics_device);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<226>");
	bb_graphics_context->m_matrixSp=0;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<227>");
	bb_graphics_SetMatrix(FLOAT(1.0),FLOAT(0.0),FLOAT(0.0),FLOAT(1.0),FLOAT(0.0),FLOAT(0.0));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<228>");
	bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<229>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<230>");
	bb_graphics_SetBlend(0);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<231>");
	bb_graphics_SetScissor(FLOAT(0.0),FLOAT(0.0),Float(bb_app_DeviceWidth()),Float(bb_app_DeviceHeight()));
	return 0;
}
int bb_graphics_EndRender(){
	DBG_ENTER("EndRender")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<235>");
	bb_graphics_renderDevice=0;
	return 0;
}
c_BBGameEvent::c_BBGameEvent(){
}
void c_BBGameEvent::mark(){
	Object::mark();
}
String c_BBGameEvent::debug(){
	String t="(BBGameEvent)\n";
	return t;
}
void bb_app_EndApp(){
	DBG_ENTER("EndApp")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<259>");
	bbError(String());
}
c_Player::c_Player(){
	m_image=0;
	m_x=0;
	m_y=0;
	m_xVel=0;
	m_yVel=0;
}
c_Player* c_Player::m_new(c_Image* t_i,int t_x,int t_y){
	DBG_ENTER("Player.new")
	c_Player *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_i,"i")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<41>");
	gc_assign(this->m_image,t_i);
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<42>");
	this->m_x=t_x;
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<43>");
	this->m_y=t_y;
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<44>");
	this->m_xVel=0;
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<45>");
	this->m_yVel=0;
	return this;
}
c_Player* c_Player::m_new2(){
	DBG_ENTER("Player.new")
	c_Player *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<33>");
	return this;
}
int c_Player::p_Update(){
	DBG_ENTER("Player.Update")
	c_Player *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<49>");
	if((bb_input_KeyDown(37))!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<50>");
		m_x=m_x-2;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<52>");
	if((bb_input_KeyDown(39))!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<53>");
		m_x=m_x+2;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<55>");
	if((bb_input_KeyDown(40))!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<56>");
		m_y=m_y+2;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<58>");
	if((bb_input_KeyDown(38))!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<59>");
		m_y=m_y-2;
	}
	return 0;
}
int c_Player::p_Draw(){
	DBG_ENTER("Player.Draw")
	c_Player *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/MonkeyTest/test.monkey<65>");
	bb_graphics_DrawImage(this->m_image,Float(m_x),Float(m_y),0);
	return 0;
}
void c_Player::mark(){
	Object::mark();
	gc_mark_q(m_image);
}
String c_Player::debug(){
	String t="(Player)\n";
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	t+=dbg_decl("xVel",&m_xVel);
	t+=dbg_decl("yVel",&m_yVel);
	t+=dbg_decl("image",&m_image);
	return t;
}
c_Sound::c_Sound(){
	m_sample=0;
}
c_Sound* c_Sound::m_new(gxtkSample* t_sample){
	DBG_ENTER("Sound.new")
	c_Sound *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_sample,"sample")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/audio.monkey<32>");
	gc_assign(this->m_sample,t_sample);
	return this;
}
c_Sound* c_Sound::m_new2(){
	DBG_ENTER("Sound.new")
	c_Sound *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/audio.monkey<29>");
	return this;
}
void c_Sound::mark(){
	Object::mark();
	gc_mark_q(m_sample);
}
String c_Sound::debug(){
	String t="(Sound)\n";
	t+=dbg_decl("sample",&m_sample);
	return t;
}
c_Sound* bb_audio_LoadSound(String t_path){
	DBG_ENTER("LoadSound")
	DBG_LOCAL(t_path,"path")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/audio.monkey<47>");
	gxtkSample* t_sample=bb_audio_device->LoadSample(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_sample,"sample")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/audio.monkey<48>");
	if((t_sample)!=0){
		DBG_BLOCK();
		c_Sound* t_=(new c_Sound)->m_new(t_sample);
		return t_;
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/audio.monkey<49>");
	return 0;
}
int bb_audio_SetChannelVolume(int t_channel,Float t_volume){
	DBG_ENTER("SetChannelVolume")
	DBG_LOCAL(t_channel,"channel")
	DBG_LOCAL(t_volume,"volume")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/audio.monkey<73>");
	bb_audio_device->SetVolume(t_channel,t_volume);
	return 0;
}
int bb_audio_PlaySound(c_Sound* t_sound,int t_channel,int t_flags){
	DBG_ENTER("PlaySound")
	DBG_LOCAL(t_sound,"sound")
	DBG_LOCAL(t_channel,"channel")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/audio.monkey<53>");
	if(((t_sound)!=0) && ((t_sound->m_sample)!=0)){
		DBG_BLOCK();
		bb_audio_device->PlaySample(t_sound->m_sample,t_channel,t_flags);
	}
	return 0;
}
int bb_app__updateRate;
void bb_app_SetUpdateRate(int t_hertz){
	DBG_ENTER("SetUpdateRate")
	DBG_LOCAL(t_hertz,"hertz")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<224>");
	bb_app__updateRate=t_hertz;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/app.monkey<225>");
	bb_app__game->SetUpdateRate(t_hertz);
}
int bb_input_KeyDown(int t_key){
	DBG_ENTER("KeyDown")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/input.monkey<40>");
	int t_=((bb_input_device->p_KeyDown(t_key))?1:0);
	return t_;
}
int bb_graphics_DebugRenderDevice(){
	DBG_ENTER("DebugRenderDevice")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<53>");
	if(!((bb_graphics_renderDevice)!=0)){
		DBG_BLOCK();
		bbError(String(L"Rendering operations can only be performed inside OnRender",58));
	}
	return 0;
}
int bb_graphics_Cls(Float t_r,Float t_g,Float t_b){
	DBG_ENTER("Cls")
	DBG_LOCAL(t_r,"r")
	DBG_LOCAL(t_g,"g")
	DBG_LOCAL(t_b,"b")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<378>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<380>");
	bb_graphics_renderDevice->Cls(t_r,t_g,t_b);
	return 0;
}
int bb_graphics_DrawImage(c_Image* t_image,Float t_x,Float t_y,int t_frame){
	DBG_ENTER("DrawImage")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<452>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<453>");
	if(t_frame<0 || t_frame>=t_image->m_frames.Length()){
		DBG_BLOCK();
		bbError(String(L"Invalid image frame",19));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<456>");
	c_Frame* t_f=t_image->m_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<458>");
	bb_graphics_context->p_Validate();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<460>");
	if((t_image->m_flags&65536)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<461>");
		bb_graphics_renderDevice->DrawSurface(t_image->m_surface,t_x-t_image->m_tx,t_y-t_image->m_ty);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<463>");
		bb_graphics_renderDevice->DrawSurface2(t_image->m_surface,t_x-t_image->m_tx,t_y-t_image->m_ty,t_f->m_x,t_f->m_y,t_image->m_width,t_image->m_height);
	}
	return 0;
}
int bb_graphics_PushMatrix(){
	DBG_ENTER("PushMatrix")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<333>");
	int t_sp=bb_graphics_context->m_matrixSp;
	DBG_LOCAL(t_sp,"sp")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<334>");
	if(t_sp==bb_graphics_context->m_matrixStack.Length()){
		DBG_BLOCK();
		gc_assign(bb_graphics_context->m_matrixStack,bb_graphics_context->m_matrixStack.Resize(t_sp*2));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<335>");
	bb_graphics_context->m_matrixStack.At(t_sp+0)=bb_graphics_context->m_ix;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<336>");
	bb_graphics_context->m_matrixStack.At(t_sp+1)=bb_graphics_context->m_iy;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<337>");
	bb_graphics_context->m_matrixStack.At(t_sp+2)=bb_graphics_context->m_jx;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<338>");
	bb_graphics_context->m_matrixStack.At(t_sp+3)=bb_graphics_context->m_jy;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<339>");
	bb_graphics_context->m_matrixStack.At(t_sp+4)=bb_graphics_context->m_tx;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<340>");
	bb_graphics_context->m_matrixStack.At(t_sp+5)=bb_graphics_context->m_ty;
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<341>");
	bb_graphics_context->m_matrixSp=t_sp+6;
	return 0;
}
int bb_graphics_Transform(Float t_ix,Float t_iy,Float t_jx,Float t_jy,Float t_tx,Float t_ty){
	DBG_ENTER("Transform")
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_LOCAL(t_jx,"jx")
	DBG_LOCAL(t_jy,"jy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<355>");
	Float t_ix2=t_ix*bb_graphics_context->m_ix+t_iy*bb_graphics_context->m_jx;
	DBG_LOCAL(t_ix2,"ix2")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<356>");
	Float t_iy2=t_ix*bb_graphics_context->m_iy+t_iy*bb_graphics_context->m_jy;
	DBG_LOCAL(t_iy2,"iy2")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<357>");
	Float t_jx2=t_jx*bb_graphics_context->m_ix+t_jy*bb_graphics_context->m_jx;
	DBG_LOCAL(t_jx2,"jx2")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<358>");
	Float t_jy2=t_jx*bb_graphics_context->m_iy+t_jy*bb_graphics_context->m_jy;
	DBG_LOCAL(t_jy2,"jy2")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<359>");
	Float t_tx2=t_tx*bb_graphics_context->m_ix+t_ty*bb_graphics_context->m_jx+bb_graphics_context->m_tx;
	DBG_LOCAL(t_tx2,"tx2")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<360>");
	Float t_ty2=t_tx*bb_graphics_context->m_iy+t_ty*bb_graphics_context->m_jy+bb_graphics_context->m_ty;
	DBG_LOCAL(t_ty2,"ty2")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<361>");
	bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
	return 0;
}
int bb_graphics_Transform2(Array<Float > t_m){
	DBG_ENTER("Transform")
	DBG_LOCAL(t_m,"m")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<351>");
	bb_graphics_Transform(t_m.At(0),t_m.At(1),t_m.At(2),t_m.At(3),t_m.At(4),t_m.At(5));
	return 0;
}
int bb_graphics_Translate(Float t_x,Float t_y){
	DBG_ENTER("Translate")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<365>");
	bb_graphics_Transform(FLOAT(1.0),FLOAT(0.0),FLOAT(0.0),FLOAT(1.0),t_x,t_y);
	return 0;
}
int bb_graphics_Rotate(Float t_angle){
	DBG_ENTER("Rotate")
	DBG_LOCAL(t_angle,"angle")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<373>");
	bb_graphics_Transform((Float)cos((t_angle)*D2R),-(Float)sin((t_angle)*D2R),(Float)sin((t_angle)*D2R),(Float)cos((t_angle)*D2R),FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int bb_graphics_Scale(Float t_x,Float t_y){
	DBG_ENTER("Scale")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<369>");
	bb_graphics_Transform(t_x,FLOAT(0.0),FLOAT(0.0),t_y,FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int bb_graphics_PopMatrix(){
	DBG_ENTER("PopMatrix")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<345>");
	int t_sp=bb_graphics_context->m_matrixSp-6;
	DBG_LOCAL(t_sp,"sp")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<346>");
	bb_graphics_SetMatrix(bb_graphics_context->m_matrixStack.At(t_sp+0),bb_graphics_context->m_matrixStack.At(t_sp+1),bb_graphics_context->m_matrixStack.At(t_sp+2),bb_graphics_context->m_matrixStack.At(t_sp+3),bb_graphics_context->m_matrixStack.At(t_sp+4),bb_graphics_context->m_matrixStack.At(t_sp+5));
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<347>");
	bb_graphics_context->m_matrixSp=t_sp;
	return 0;
}
int bb_graphics_DrawImage2(c_Image* t_image,Float t_x,Float t_y,Float t_rotation,Float t_scaleX,Float t_scaleY,int t_frame){
	DBG_ENTER("DrawImage")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_rotation,"rotation")
	DBG_LOCAL(t_scaleX,"scaleX")
	DBG_LOCAL(t_scaleY,"scaleY")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<470>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<471>");
	if(t_frame<0 || t_frame>=t_image->m_frames.Length()){
		DBG_BLOCK();
		bbError(String(L"Invalid image frame",19));
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<474>");
	c_Frame* t_f=t_image->m_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<476>");
	bb_graphics_PushMatrix();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<478>");
	bb_graphics_Translate(t_x,t_y);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<479>");
	bb_graphics_Rotate(t_rotation);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<480>");
	bb_graphics_Scale(t_scaleX,t_scaleY);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<482>");
	bb_graphics_Translate(-t_image->m_tx,-t_image->m_ty);
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<484>");
	bb_graphics_context->p_Validate();
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<486>");
	if((t_image->m_flags&65536)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<487>");
		bb_graphics_renderDevice->DrawSurface(t_image->m_surface,FLOAT(0.0),FLOAT(0.0));
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<489>");
		bb_graphics_renderDevice->DrawSurface2(t_image->m_surface,FLOAT(0.0),FLOAT(0.0),t_f->m_x,t_f->m_y,t_image->m_width,t_image->m_height);
	}
	DBG_INFO("/Applications/MonkeyXFree84f/modules/mojo/graphics.monkey<492>");
	bb_graphics_PopMatrix();
	return 0;
}
int bbInit(){
	GC_CTOR
	bb_app__app=0;
	DBG_GLOBAL("_app",&bb_app__app);
	bb_app__delegate=0;
	DBG_GLOBAL("_delegate",&bb_app__delegate);
	bb_app__game=BBGame::Game();
	DBG_GLOBAL("_game",&bb_app__game);
	bb_graphics_device=0;
	DBG_GLOBAL("device",&bb_graphics_device);
	bb_graphics_context=(new c_GraphicsContext)->m_new();
	DBG_GLOBAL("context",&bb_graphics_context);
	c_Image::m_DefaultFlags=0;
	DBG_GLOBAL("DefaultFlags",&c_Image::m_DefaultFlags);
	bb_audio_device=0;
	DBG_GLOBAL("device",&bb_audio_device);
	bb_input_device=0;
	DBG_GLOBAL("device",&bb_input_device);
	bb_app__devWidth=0;
	DBG_GLOBAL("_devWidth",&bb_app__devWidth);
	bb_app__devHeight=0;
	DBG_GLOBAL("_devHeight",&bb_app__devHeight);
	bb_app__displayModes=Array<c_DisplayMode* >();
	DBG_GLOBAL("_displayModes",&bb_app__displayModes);
	bb_app__desktopMode=0;
	DBG_GLOBAL("_desktopMode",&bb_app__desktopMode);
	bb_graphics_renderDevice=0;
	DBG_GLOBAL("renderDevice",&bb_graphics_renderDevice);
	bb_app__updateRate=0;
	DBG_GLOBAL("_updateRate",&bb_app__updateRate);
	return 0;
}
void gc_mark(){
	gc_mark_q(bb_app__app);
	gc_mark_q(bb_app__delegate);
	gc_mark_q(bb_graphics_device);
	gc_mark_q(bb_graphics_context);
	gc_mark_q(bb_audio_device);
	gc_mark_q(bb_input_device);
	gc_mark_q(bb_app__displayModes);
	gc_mark_q(bb_app__desktopMode);
	gc_mark_q(bb_graphics_renderDevice);
}
//${TRANSCODE_END}

int main( int argc,const char *argv[] ){

	BBMonkeyGame::Main( argc,argv );
}
