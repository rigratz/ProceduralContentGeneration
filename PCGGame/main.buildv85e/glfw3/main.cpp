
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
#define CFG_IGNITION_ALLOW_SET_ALPHA_FOR_EACH_OBJECT 1
#define CFG_IGNITION_ALLOW_SET_BLEND_FOR_EACH_OBJECT 1
#define CFG_IGNITION_ALLOW_SET_COLOR_FOR_EACH_OBJECT 1
#define CFG_IGNITION_DEBUG_TOOLS 0
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
	
	String _baseDir;
	String _internalDir;
	
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

	if( !_baseDir.Length() ){
	
		String appPath;

#if _WIN32
		WCHAR buf[MAX_PATH+1];
		GetModuleFileNameW( GetModuleHandleW(0),buf,MAX_PATH );
		buf[MAX_PATH]=0;appPath=String( buf ).Replace( "\\","/" );

#elif __APPLE__

		char buf[PATH_MAX+1];
		uint32_t size=sizeof( buf );
		_NSGetExecutablePath( buf,&size );
		buf[PATH_MAX]=0;appPath=String( buf ).Replace( "/./","/" );
	
#elif __linux
		char lnk[PATH_MAX+1],buf[PATH_MAX];
		sprintf( lnk,"/proc/%i/exe",getpid() );
		int n=readlink( lnk,buf,PATH_MAX );
		if( n<0 || n>=PATH_MAX ) abort();
		appPath=String( buf,n );

#endif
		int i=appPath.FindLast( "/" );if( i==-1 ) abort();
		_baseDir=appPath.Slice( 0,i );
		
#if __APPLE__
		if( _baseDir.EndsWith( ".app/Contents/MacOS" ) ) _baseDir=_baseDir.Slice( 0,-5 )+"Resources";
#endif
//		bbPrint( String( "_baseDir=" )+_baseDir );
	}
	
	if( !path.StartsWith( "monkey:" ) ){
		return path;
	}else if( path.StartsWith( "monkey://data/" ) ){
		return _baseDir+"/data/"+path.Slice( 14 );
	}else if( path.StartsWith( "monkey://internal/" ) ){
		if( !_internalDir.Length() ){
#ifdef CFG_GLFW_APP_LABEL

#if _WIN32
			_internalDir=String( getenv( "APPDATA" ) );
#elif __APPLE__
			_internalDir=String( getenv( "HOME" ) )+"/Library/Application Support";
#elif __linux
			_internalDir=String( getenv( "HOME" ) )+"/.config";
			mkdir( _internalDir.ToCString<char>(),0777 );
#endif

#ifdef CFG_GLFW_APP_PUBLISHER
			_internalDir=_internalDir+"/"+_STRINGIZE( CFG_GLFW_APP_PUBLISHER );
#if _WIN32
			_wmkdir( _internalDir.ToCString<wchar_t>() );
#else
			mkdir( _internalDir.ToCString<char>(),0777 );
#endif
#endif

			_internalDir=_internalDir+"/"+_STRINGIZE( CFG_GLFW_APP_LABEL );
#if _WIN32
			_wmkdir( _internalDir.ToCString<wchar_t>() );
#else
			mkdir( _internalDir.ToCString<char>(),0777 );
#endif

#else
			_internalDir=_baseDir+"/internal";
#endif			
//			bbPrint( String( "_internalDir=" )+_internalDir );
		}
		return _internalDir+"/"+path.Slice( 18 );
	}else if( path.StartsWith( "monkey://external/" ) ){
		return _baseDir+"/external/"+path.Slice( 18 );
	}
	return "";
}

/*
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
*/

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
class c_iApp;
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
class c_iObject;
class c_iScene;
class c_iDeltaTimer;
class c_iEngine;
class c_MenuScene;
class c_GameplayScene;
class c_NoiseTestScene;
class c_Path;
class c_PathStep;
class c_List;
class c_Node2;
class c_HeadNode;
class c_iEngineObject;
class c_iPlayfield;
class c_iList;
class c_iStack2D;
class c_iLayer;
class c_iList2;
class c_IMover;
class c_iGraph;
class c_iLayerObject;
class c_iList3;
class c_iGuiObject;
class c_iList4;
class c_iSystemGui;
class c_iTask;
class c_iList5;
class c_iCollision;
class c_iList6;
class c_BattleScene;
class c_iLayerSprite;
class c_iLayerGroup;
class c_iContentManager;
class c_iContentObject;
class c_Player;
class c_Animation;
class c_Sound;
class c_Level;
class c_Point;
class c_AStarSearch;
class c_AStarNode;
class c_Stack2;
class c_Stack3;
class c_Stack4;
class c_BoundingRect;
class c_SimplexNoise;
class c_Enumerator;
class c_iConfig;
class c_iVector2d;
class c_iControlSet;
class c_App : public Object{
	public:
	c_App();
	c_App* m_new();
	virtual int p_OnResize();
	virtual int p_OnCreate();
	virtual int p_OnSuspend();
	virtual int p_OnResume();
	virtual int p_OnUpdate();
	virtual int p_OnLoading();
	virtual int p_OnRender();
	virtual int p_OnClose();
	virtual int p_OnBack();
	void mark();
	String debug();
};
String dbg_type(c_App**p){return "App";}
class c_iApp : public c_App{
	public:
	c_iApp();
	c_iApp* m_new();
	int p_OnBack();
	int p_OnClose();
	int p_OnLoading();
	int p_OnRender();
	int p_OnResize();
	int p_OnResume();
	int p_OnSuspend();
	int p_OnUpdate();
	void mark();
	String debug();
};
String dbg_type(c_iApp**p){return "iApp";}
class c_Game : public c_iApp{
	public:
	c_Game();
	c_Game* m_new();
	int p_OnCreate();
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
	int p_Width();
	int p_Height();
	int p_Frames();
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
	void p_Reset();
	bool p_KeyDown(int);
	Float p_TouchX(int);
	Float p_TouchY(int);
	int p_KeyHit(int);
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
class c_iObject : public Object{
	public:
	int m__enabled;
	bool m__tattoo;
	int m__inPool;
	c_iObject();
	virtual int p_OnDisabled();
	c_iObject* m_new();
	int p_OnDisable();
	void p_Disable();
	void mark();
	String debug();
};
String dbg_type(c_iObject**p){return "iObject";}
class c_iScene : public c_iObject{
	public:
	int m__visible;
	bool m__started;
	bool m__cold;
	bool m__paused;
	c_iScene();
	int p_OnBack();
	int p_OnClose();
	int p_OnLoading();
	virtual int p_OnRender();
	virtual void p_Render();
	virtual int p_OnResize();
	virtual void p_Resume();
	int p_OnResume();
	void p_Suspend();
	int p_OnSuspend();
	virtual int p_OnStop();
	virtual void p_Set2();
	virtual int p_OnColdStart();
	virtual int p_OnStart();
	int p_OnPaused();
	virtual int p_OnUpdate();
	virtual void p_Update();
	virtual int p_OnCreate();
	void p_SystemInit();
	c_iScene* m_new();
	void mark();
	String debug();
};
String dbg_type(c_iScene**p){return "iScene";}
extern c_iScene* bb_app2_iCurrentScene;
void bb_app2_iBack();
void bb_app2_iClose();
void bb_app2_iLoading();
void bb_app2_iRender();
void bb_app2_iResize();
void bb_app2_iResume();
void bb_app2_iSuspend();
class c_iDeltaTimer : public Object{
	public:
	Float m__targetFPS;
	int m__lastTicks;
	Float m__frameTime;
	Float m__elapsedTime;
	Float m__timeScale;
	Float m__deltaTime;
	Float m__elapsedDelta;
	c_iDeltaTimer();
	c_iDeltaTimer* m_new(Float);
	c_iDeltaTimer* m_new2();
	void p_Update();
	void p_Resume();
	void mark();
	String debug();
};
String dbg_type(c_iDeltaTimer**p){return "iDeltaTimer";}
int bb_app_Millisecs();
extern c_iDeltaTimer* bb_app2_iDT;
extern int bb_app2_iSpeed;
extern c_iScene* bb_app2_iNextScene;
int bb_input_ResetInput();
void bb_app2_iUpdate();
extern int bb_random_Seed;
class c_iEngine : public c_iScene{
	public:
	int m__timeCode;
	bool m__autoCls;
	Float m__clsRed;
	Float m__colorFade;
	Float m__clsGreen;
	Float m__clsBlue;
	bool m__renderToPlayfield;
	c_iList* m__playfieldList;
	c_iPlayfield* m__currentPlayfield;
	Float m__alphaFade;
	bool m__borders;
	bool m__showSystemGui;
	c_iList5* m__taskList;
	int m__countDown;
	int m__scoreCount;
	c_iPlayfield* m__playfieldPointer;
	c_iList3* m__collisionReadList;
	c_iLayer* m__layerPointer;
	c_iList3* m__collisionWriteList;
	c_iEngine();
	c_iEngine* m_new();
	int p_OnColdStart();
	int p_OnResize();
	Float p_AlphaFade();
	void p_AlphaFade2(Float);
	int p_SetAlpha(Float);
	Float p_ColorFade();
	void p_ColorFade2(Float);
	int p_SetColor(Float,Float,Float);
	int p_OnTopRender();
	void p_RenderPlayfields();
	void p_RenderSystemGui();
	int p_DebugRender();
	void p_Render();
	void p_Resume();
	void p_Set2();
	int p_OnGuiPageChange(c_iPlayfield*);
	void p_UpdateSystemGui();
	void p_UpdatePlayfields();
	void p_UpdateService();
	void p_UpdateCollisions();
	void p_UpdateLogic();
	void p_UpdateFixed();
	void p_Update();
	void mark();
	String debug();
};
String dbg_type(c_iEngine**p){return "iEngine";}
class c_MenuScene : public c_iEngine{
	public:
	c_MenuScene();
	c_MenuScene* m_new();
	int p_OnCreate();
	int p_OnRender();
	int p_OnStart();
	int p_OnStop();
	int p_OnUpdate();
	void mark();
	String debug();
};
String dbg_type(c_MenuScene**p){return "MenuScene";}
extern c_MenuScene* bb_main_menu;
class c_GameplayScene : public c_iEngine{
	public:
	c_iPlayfield* m_playfield;
	c_iLayer* m_layer;
	c_iLayerSprite* m_sprite1;
	c_Player* m_p1;
	c_Sound* m_music;
	c_Level* m_room;
	c_BoundingRect* m_bBox;
	c_GameplayScene();
	c_GameplayScene* m_new();
	int p_OnCreate();
	int p_OnRender();
	int p_OnStart();
	int p_OnStop();
	int p_checkCameraBounds();
	int p_OnUpdate();
	void mark();
	String debug();
};
String dbg_type(c_GameplayScene**p){return "GameplayScene";}
extern c_GameplayScene* bb_main_gameplay;
class c_NoiseTestScene : public c_iEngine{
	public:
	c_iPlayfield* m_playfieldN;
	int m_mapWidth;
	int m_mapHeight;
	c_iLayer* m_layer;
	c_iLayerSprite* m_sprite1;
	Array<c_Point* > m_deepWaterTiles;
	Array<c_Point* > m_shallowWaterTiles;
	Array<c_Point* > m_beachTiles;
	Array<c_Point* > m_lightGrassTiles;
	Array<c_Point* > m_heavyGrassTiles;
	Array<c_Point* > m_swampTiles;
	Array<c_Point* > m_forestTiles;
	Array<c_Point* > m_desertTiles;
	Array<c_Point* > m_mountainTiles;
	Array<c_Point* > m_darkSnowMountainTiles;
	Array<c_Point* > m_lightSnowMountainTiles;
	Array<c_Point* > m_riverTiles;
	Array<c_Point* > m_caveEntranceTiles;
	Array<Array<int > > m_biomes;
	Array<Array<int > > m_enemyPlacement;
	Array<Array<Float > > m_noiseMap;
	Array<Array<Float > > m_moisture;
	int m_tileCounter;
	int m_startX;
	int m_startY;
	c_BoundingRect* m_bBox;
	Array<c_Level* > m_caves;
	int m_caveX;
	int m_caveY;
	c_NoiseTestScene();
	c_NoiseTestScene* m_new();
	int p_OnCreate();
	int p_drawNoiseMap(int,int);
	int p_drawEnemies(int,int);
	int p_OnRender();
	int p_determineBiomes(Array<Array<Float > >,Array<Array<Float > >,int,int);
	int p_processBiomes();
	int p_detailBiomes();
	int p_makeLake(int,int);
	int p_makeRiver(int,int);
	int p_makeRivers(int);
	int p_selectRandomStartPoint();
	int p_makeCaves(int);
	int p_determineEnemyType(Array<Array<int > >,Array<Array<int > >);
	int p_placeEnemies(int,int);
	int p_OnStart();
	int p_OnStop();
	bool p_isWalkable(int);
	int p_checkCameraBounds();
	int p_OnUpdate();
	void mark();
	String debug();
};
String dbg_type(c_NoiseTestScene**p){return "NoiseTestScene";}
extern c_NoiseTestScene* bb_main_noiseTest;
class c_Path : public Object{
	public:
	c_List* m_steps;
	c_Path();
	c_Path* m_new();
	int p_prependStep(int,int);
	int p_pathLength();
	void mark();
	String debug();
};
String dbg_type(c_Path**p){return "Path";}
class c_PathStep : public Object{
	public:
	int m_x;
	int m_y;
	c_PathStep();
	c_PathStep* m_new(int,int);
	c_PathStep* m_new2();
	int p_getX();
	int p_getY();
	void mark();
	String debug();
};
String dbg_type(c_PathStep**p){return "PathStep";}
class c_List : public Object{
	public:
	c_Node2* m__head;
	c_List();
	c_List* m_new();
	c_Node2* p_AddLast(c_PathStep*);
	c_List* m_new2(Array<c_PathStep* >);
	c_Node2* p_AddFirst(c_PathStep*);
	int p_Count();
	c_Enumerator* p_ObjectEnumerator();
	void mark();
	String debug();
};
String dbg_type(c_List**p){return "List";}
class c_Node2 : public Object{
	public:
	c_Node2* m__succ;
	c_Node2* m__pred;
	c_PathStep* m__data;
	c_Node2();
	c_Node2* m_new(c_Node2*,c_Node2*,c_PathStep*);
	c_Node2* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Node2**p){return "Node";}
class c_HeadNode : public c_Node2{
	public:
	c_HeadNode();
	c_HeadNode* m_new();
	void mark();
	String debug();
};
String dbg_type(c_HeadNode**p){return "HeadNode";}
extern int bb_app__updateRate;
int bb_app_UpdateRate();
void bb_app_SetUpdateRate(int);
void bb_app2_iStart(int);
void bb_app2_iStart2(c_iScene*);
void bb_app2_iStart3(c_iScene*,int);
extern c_iEngine* bb_globals_iEnginePointer;
int bb_functions_iTimeCode();
Array<Float > bb_graphics_GetScissor();
int bb_graphics_GetScissor2(Array<Float >);
int bb_graphics_DebugRenderDevice();
int bb_graphics_Cls(Float,Float,Float);
void bb_gfx_iCls(Float,Float,Float);
class c_iEngineObject : public c_iObject{
	public:
	bool m__autoDestroy;
	c_iEngineObject();
	c_iEngineObject* m_new();
	virtual int p_OnRemove();
	virtual void p_Remove();
	static c_iList6* m_destroyList;
	int p_OnDestroy();
	virtual void p_Destroy();
	static void m_DestroyList2();
	void mark();
	String debug();
};
String dbg_type(c_iEngineObject**p){return "iEngineObject";}
class c_iPlayfield : public c_iEngineObject{
	public:
	Float m__alphaFade;
	Float m__colorFade;
	bool m__visible;
	Float m__x;
	Float m__y;
	int m__vwidth;
	Float m__scaleX;
	int m__vheight;
	Float m__scaleY;
	bool m__autoCls;
	Float m__clsRed;
	Float m__clsGreen;
	Float m__clsBlue;
	Float m__zoomPointX;
	Float m__zoomPointY;
	Float m__zoomX;
	Float m__zoomY;
	Float m__rotation;
	bool m__backgroundRender;
	Float m__cameraX;
	Float m__cameraY;
	c_iList2* m__layerList;
	c_iLayer* m__currentLayer;
	int m__height;
	int m__width;
	bool m__guiEnabled;
	Array<c_iList4* > m__guiList;
	int m__guiPage;
	c_iGuiObject* m__guiLastObject;
	int m__nextGuiPage;
	Float m__z;
	c_iPlayfield();
	virtual int p_SetAlpha(Float);
	virtual int p_SetColor(Float,Float,Float);
	int p_OnBackgroundRender();
	int p_OnRender();
	Float p_PositionX();
	void p_PositionX2(Float);
	Float p_CameraX();
	void p_CameraX2(Float);
	Float p_PositionY();
	void p_PositionY2(Float);
	Float p_CameraY();
	void p_CameraY2(Float);
	Float p_VWidth();
	Float p_ZoomPointX();
	void p_ZoomPointX2(Float);
	Float p_VHeight();
	Float p_ZoomPointY();
	void p_ZoomPointY2(Float);
	void p_ZoomPoint(Float,Float);
	void p_VHeight2(int);
	void p_ZoomPoint2();
	void p_VWidth2(int);
	int p_OnTopRender();
	void p_RenderGui();
	void p_Render();
	int p_GuiPage();
	void p_InitGuiPages(int);
	int p_OnGuiPageChange2();
	void p_GuiPage2(int);
	virtual void p_Set2();
	int p_OnCreate();
	void p_SystemInit();
	c_iPlayfield* m_new();
	c_iPlayfield* m_new2(int,int);
	c_iPlayfield* m_new3(Float,Float,int,int);
	void p_UpdateGui();
	int p_OnUpdate();
	void p_Update();
	void p_AttachLast();
	void p_AttachLast2(c_iEngine*);
	void p_AutoCls(int,int,int);
	void p_AutoCls2(bool);
	Float p_Width();
	void p_Width2(Float);
	Float p_Height();
	void p_Height2(Float);
	void p_Position(Float,Float);
	void p_Position2(Float,Float,Float);
	Float p_AlphaFade();
	void p_AlphaFade2(Float);
	Float p_ZoomX();
	void p_ZoomX2(Float);
	Float p_ZoomY();
	void p_ZoomY2(Float);
	void p_Remove();
	void mark();
	String debug();
};
String dbg_type(c_iPlayfield**p){return "iPlayfield";}
class c_iList : public Object{
	public:
	int m__length;
	int m__index;
	Array<c_iPlayfield* > m__data;
	c_iList();
	c_iList* m_new();
	int p_Length();
	c_iPlayfield* p_First();
	c_iPlayfield* p_Ascend();
	c_iPlayfield* p_Get(int);
	void p_AddLast2(c_iPlayfield*);
	int p_Position3(c_iPlayfield*);
	void p_RemoveFromIndex(int);
	void p_Remove2(c_iPlayfield*);
	void mark();
	String debug();
};
String dbg_type(c_iList**p){return "iList";}
class c_iStack2D : public Object{
	public:
	Float m__alpha;
	int m__blend;
	Array<Float > m__color;
	Array<Float > m__matrix;
	Array<Float > m__scissor;
	c_iStack2D();
	static int m__length;
	static Array<c_iStack2D* > m__data;
	c_iStack2D* m_new();
	static c_iStack2D* m_Push();
	static void m_Pop();
	void mark();
	String debug();
};
String dbg_type(c_iStack2D**p){return "iStack2D";}
Float bb_graphics_GetAlpha();
int bb_graphics_GetBlend();
Array<Float > bb_graphics_GetColor();
int bb_graphics_GetColor2(Array<Float >);
Array<Float > bb_graphics_GetMatrix();
int bb_graphics_GetMatrix2(Array<Float >);
void bb_gfx_iSetScissor(Float,Float,Float,Float);
int bb_graphics_Transform(Float,Float,Float,Float,Float,Float);
int bb_graphics_Transform2(Array<Float >);
int bb_graphics_Scale(Float,Float);
int bb_graphics_Translate(Float,Float);
extern int bb_globals_iRotation;
int bb_graphics_Rotate(Float);
class c_iLayer : public c_iEngineObject{
	public:
	bool m__visible;
	c_iStack2D* m__stack2D;
	Float m__cameraSpeedX;
	Float m__cameraSpeedY;
	Array<Array<c_iList3* > > m__objectList;
	int m__blockWidth;
	int m__blockWidthExtra;
	int m__blockHeight;
	int m__blockHeightExtra;
	int m__lastUpdate;
	c_iConfig* m__loaderCache;
	Float m__alphaFade;
	Float m__colorFade;
	c_iLayer();
	Float p_CameraSpeedX();
	void p_CameraSpeedX2(Float);
	Float p_CameraSpeedY();
	void p_CameraSpeedY2(Float);
	int p_OnRender();
	bool p_Grid();
	int p_GridWidth();
	int p_GridHeight();
	void p_RenderLayerObjects();
	int p_OnTopRender();
	void p_Render();
	void p_UpdateWorldXY();
	void p_UpdateLayerObjects();
	int p_OnUpdate();
	void p_Update();
	void p_Set2();
	void p_InitGrid(int,int);
	int p_OnCreate();
	void p_SystemInit();
	c_iLayer* m_new();
	void p_AttachLast();
	void p_AttachLast3(c_iPlayfield*);
	void p_Destroy();
	void p_Remove();
	void mark();
	String debug();
};
String dbg_type(c_iLayer**p){return "iLayer";}
class c_iList2 : public Object{
	public:
	int m__index;
	int m__length;
	Array<c_iLayer* > m__data;
	c_iList2();
	c_iList2* m_new();
	c_iLayer* p_First();
	c_iLayer* p_Ascend();
	void p_AddLast3(c_iLayer*);
	int p_Position4(c_iLayer*);
	void p_RemoveFromIndex(int);
	void p_Remove3(c_iLayer*);
	void mark();
	String debug();
};
String dbg_type(c_iList2**p){return "iList";}
class c_IMover : public virtual gc_interface{
	public:
};
class c_iGraph : public c_iEngineObject,public virtual c_IMover{
	public:
	bool m__visible;
	bool m__ghost;
	int m__height;
	Float m__scaleY;
	Float m__scaleX;
	Float m__rotation;
	int m__width;
	Float m__x;
	Float m__y;
	Float m__z;
	c_iVector2d* m__mcPosition;
	c_iVector2d* m__mcVelocity;
	int m__ghostBlend;
	Float m__ghostAlpha;
	Float m__ghostRed;
	Float m__ghostGreen;
	Float m__ghostBlue;
	int m__blend;
	Float m__alpha;
	Float m__red;
	Float m__green;
	Float m__blue;
	c_iGraph();
	virtual Float p_Height();
	void p_Height2(Float);
	Float p_ScaleX();
	void p_ScaleX2(Float);
	Float p_Rotation();
	void p_Rotation2(Float);
	virtual Float p_Width();
	void p_Width2(Float);
	Float p_PositionX();
	void p_PositionX2(Float);
	void p_PositionX3(Float,c_iGraph*);
	Float p_PositionY();
	void p_PositionY2(Float);
	void p_PositionY3(Float,c_iGraph*);
	c_iGraph* m_new();
	Float p_PositionZ();
	void p_PositionZ2(Float);
	void p_PositionZ3(Float,c_iGraph*);
	void p_Destroy();
	Float p_ScaleY();
	void p_ScaleY2(Float);
	void mark();
	String debug();
};
String dbg_type(c_iGraph**p){return "iGraph";}
class c_iLayerObject : public c_iGraph{
	public:
	Float m__worldHeight;
	Float m__worldScaleX;
	Float m__worldScaleY;
	Float m__worldRotation;
	Float m__worldWidth;
	Float m__worldX;
	Float m__worldY;
	c_iLayerObject* m__parent;
	c_iLayer* m__layer;
	c_iLayerObject* m__scoreCollector;
	Float m__cosine;
	Float m__cosineRadius;
	Float m__sine;
	Float m__sineRadius;
	int m__stamina;
	bool m__invincible;
	int m__hits;
	bool m__scoreCount;
	int m__scoreBoard;
	int m__points;
	bool m__countDown;
	bool m__collisionRead;
	bool m__collisionWrite;
	int m__column;
	int m__row;
	c_iConfig* m__loaderCache;
	Array<int > m__collisionMask;
	Array<c_iControlSet* > m__control;
	Array<String > m__debugInfo;
	c_iLayerObject();
	virtual void p_Render();
	virtual void p_UpdateWorldXY();
	virtual void p_Update();
	int p_OnUpdate();
	int p_OnDisabled();
	static c_iLayer* m_AutoLayer();
	c_iLayer* p_Layer();
	void p_Layer2(c_iLayer*);
	Float p_Cosine();
	void p_Cosine2(Float);
	Float p_CosineRadius();
	void p_CosineRadius2(Float);
	Float p_RenderX();
	Float p_Sine();
	void p_Sine2(Float);
	Float p_SineRadius();
	void p_SineRadius2(Float);
	Float p_RenderY();
	int p_CollisionMethod(c_iLayerObject*);
	int p_Collides(c_iLayerObject*);
	int p_OnOutro();
	int p_OnRemove();
	void p_Remove();
	void p_ScoreSystem(c_iLayerObject*);
	int p_OnCollision(c_iLayerObject*);
	c_iLayerObject* m_new();
	int p_OnAttach();
	void p_Attach();
	void p_AttachLast();
	void p_AttachLast4(c_iLayer*);
	c_iLayerObject* p_Parent();
	void p_Parent2(c_iLayerObject*);
	void p_AttachLast5(c_iLayerGroup*);
	void p_Position(Float,Float);
	void p_Position5(Float,Float,c_iLayerObject*);
	void p_Position2(Float,Float,Float);
	void p_Position6(Float,Float,Float,c_iLayerObject*);
	void p_Destroy();
	int p_SetAlpha(Float);
	int p_SetColor(Float,Float,Float);
	void p_RenderDebugInfo();
	void p_ClearDebugInfo();
	void p_RenderObject(c_iLayerObject*);
	void p_RenderList(c_iList3*);
	bool p_TestForRender();
	void p_UpdateObject(c_iLayerObject*);
	void p_UpdateList(c_iList3*);
	void p_UpdateWorldXYObject(c_iLayerObject*);
	void p_UpdateWorldXYList(c_iList3*);
	void mark();
	String debug();
};
String dbg_type(c_iLayerObject**p){return "iLayerObject";}
class c_iList3 : public Object{
	public:
	int m__length;
	Array<c_iLayerObject* > m__data;
	int m__index;
	c_iList3();
	int p_Length();
	c_iLayerObject* p_Get(int);
	c_iLayerObject* p_First();
	c_iLayerObject* p_Ascend();
	c_iList3* m_new();
	int p_Position7(c_iLayerObject*);
	void p_RemoveFromIndex(int);
	void p_Remove4(c_iLayerObject*);
	void p_AddLast4(c_iLayerObject*);
	void mark();
	String debug();
};
String dbg_type(c_iList3**p){return "iList";}
class c_iGuiObject : public c_iGraph{
	public:
	Float m__worldHeight;
	Float m__worldScaleX;
	Float m__worldScaleY;
	Float m__worldRotation;
	Float m__worldWidth;
	Float m__worldX;
	Float m__worldY;
	bool m__multiTouch;
	int m__touchIndex;
	c_iGuiObject* m__parent;
	c_iPlayfield* m__playfield;
	int m__handleX;
	int m__handleY;
	int m__touchDown;
	bool m__over;
	bool m__rememberTopObject;
	Array<String > m__debugInfo;
	c_iGuiObject();
	void p_RenderGhost();
	void p_Render();
	static c_iGuiObject* m__topObject;
	void p_UpdateWorldXY();
	c_iGuiObject* p_Parent();
	void p_Parent3(c_iGuiObject*);
	static c_iPlayfield* m_AutoPlayfield();
	void p_Playfield(c_iPlayfield*);
	c_iPlayfield* p_Playfield2();
	Float p_TouchX(int);
	Float p_TouchY(int);
	bool p_TouchOver(int);
	bool p_GetTouchIndex();
	void p_UpdateInput();
	void p_UpdateGhost();
	void p_ClearDebugInfo();
	void p_Update();
	int p_OnUpdate();
	void p_Destroy();
	int p_OnRemove();
	void p_Remove();
	void mark();
	String debug();
};
String dbg_type(c_iGuiObject**p){return "iGuiObject";}
class c_iList4 : public Object{
	public:
	int m__index;
	int m__length;
	Array<c_iGuiObject* > m__data;
	c_iList4();
	c_iGuiObject* p_First();
	c_iGuiObject* p_Ascend();
	c_iList4* m_new();
	c_iGuiObject* p_Last();
	c_iGuiObject* p_Descend();
	int p_Position8(c_iGuiObject*);
	void p_RemoveFromIndex(int);
	void p_Remove5(c_iGuiObject*);
	void mark();
	String debug();
};
String dbg_type(c_iList4**p){return "iList";}
int bb_graphics_DrawRect(Float,Float,Float,Float);
class c_iSystemGui : public c_iPlayfield{
	public:
	c_iSystemGui();
	static c_iSystemGui* m__playfield;
	static c_iPlayfield* m_Playfield();
	static void m_GuiPage(int);
	void p_Set2();
	int p_SetAlpha(Float);
	int p_SetColor(Float,Float,Float);
	void mark();
	String debug();
};
String dbg_type(c_iSystemGui**p){return "iSystemGui";}
class c_iTask : public c_iObject{
	public:
	int m__visible;
	bool m__cold;
	bool m__started;
	c_iTask();
	int p_OnRender();
	void p_Render();
	static void m_Render(c_iList5*);
	int p_OnUpdate();
	void p_Update();
	void p_SystemStart();
	int p_OnColdStart();
	int p_OnStart();
	static void m_Update(c_iList5*);
	void mark();
	String debug();
};
String dbg_type(c_iTask**p){return "iTask";}
class c_iList5 : public Object{
	public:
	int m__length;
	int m__index;
	Array<c_iTask* > m__data;
	c_iList5();
	c_iList5* m_new();
	int p_Length();
	c_iTask* p_First();
	c_iTask* p_Ascend();
	void mark();
	String debug();
};
String dbg_type(c_iList5**p){return "iList";}
extern c_iList5* bb_globals_iTaskList;
int bb_math2_Abs(int);
Float bb_math2_Abs2(Float);
extern int bb_globals_iMultiTouch;
int bb_input_TouchDown(int);
int bb_functions_iTouchDowns();
Float bb_input_TouchX(int);
Float bb_input_TouchY(int);
c_iPlayfield* bb_functions_iGetTouchPlayfield(int);
bool bb_commoncode_iPointInsideRectange(Float,Float,Float,Float,Float,Float,Float,Float,Float);
class c_iCollision : public Object{
	public:
	c_iCollision();
	static bool m_Rectangles(Float,Float,int,int,Float,Float,int,int,int);
	void mark();
	String debug();
};
String dbg_type(c_iCollision**p){return "iCollision";}
class c_iList6 : public Object{
	public:
	int m__length;
	Array<c_iEngineObject* > m__data;
	int m__index;
	c_iList6();
	c_iList6* m_new();
	void p_AddLast5(c_iEngineObject*);
	int p_Length();
	c_iEngineObject* p_First();
	int p_Position9(c_iEngineObject*);
	void p_RemoveFromIndex(int);
	void p_Remove6(c_iEngineObject*);
	c_iEngineObject* p_Ascend();
	void mark();
	String debug();
};
String dbg_type(c_iList6**p){return "iList";}
int bb_graphics_DrawImage(c_Image*,Float,Float,int);
int bb_graphics_PushMatrix();
int bb_graphics_PopMatrix();
int bb_graphics_DrawImage2(c_Image*,Float,Float,Float,Float,Float,int);
int bb_graphics_DrawText(String,Float,Float,Float,Float);
int bb_input_KeyHit(int);
class c_BattleScene : public c_iEngine{
	public:
	int m_enemyTimer;
	int m_enemyTimer2;
	int m_playerTimer;
	String m_turn;
	int m_playerTurns;
	c_Image* m_bg;
	c_BattleScene();
	c_BattleScene* m_new();
	int p_OnCreate();
	int p_OnRender();
	int p_OnStart();
	int p_OnStop();
	int p_OnUpdate();
	void mark();
	String debug();
};
String dbg_type(c_BattleScene**p){return "BattleScene";}
c_Image* bb_gfx_iLoadSprite(String,int);
c_Image* bb_gfx_iLoadSprite2(String,int,int,int);
String bb_strings_iStripExt(String);
String bb_strings_iExtractExt(String);
Array<c_Image* > bb_gfx_iLoadImage(int,int,String,int,int);
Array<c_Image* > bb_gfx_iLoadImage2(Array<c_Image* >,int,int,String,int,int);
Array<c_Image* > bb_gfx_iLoadImage3(int,int,String,int,int,int,int);
Array<c_Image* > bb_gfx_iLoadSprite3(int,int,String,int);
Array<c_Image* > bb_gfx_iLoadSprite4(int,int,String,int,int,int);
class c_iLayerSprite : public c_iLayerObject{
	public:
	Array<c_Image* > m__imagePointer;
	Array<c_Image* > m__imageSignature;
	Array<c_Image* > m__ghostImagePointer;
	Float m__frame;
	Float m__imageIndex;
	Float m__frameOffset;
	int m__animationMode;
	String m__imagePath;
	c_iLayerSprite();
	c_iLayerSprite* m_new();
	Array<c_Image* > p_ImagePointer();
	void p_ImagePointer2(c_Image*);
	bool p_Compare2(Array<c_Image* >,Array<c_Image* >);
	void p_ImagePointer3(Array<c_Image* >);
	void p_ImagePointer4(int,int,String,int,int,int);
	void p_ImagePointer5(int,int,String,int);
	void p_ImagePointer6(String,int,int,int);
	void p_ImagePointer7(String,int);
	void p_Destroy();
	Float p_ImageIndex();
	void p_ImageIndex2(Float);
	bool p_ImageLoaded();
	Float p_Height();
	Float p_ImageFrame();
	void p_Render();
	void p_Update();
	Float p_Width();
	void mark();
	String debug();
};
String dbg_type(c_iLayerSprite**p){return "iLayerSprite";}
class c_iLayerGroup : public c_iLayerObject{
	public:
	c_iList3* m__objectList;
	c_iLayerGroup();
	void p_AddItemLast(c_iLayerObject*);
	Float p_Height();
	void p_Render();
	void p_Update();
	Float p_Width();
	void p_UpdateWorldXY();
	void mark();
	String debug();
};
String dbg_type(c_iLayerGroup**p){return "iLayerGroup";}
class c_iContentManager : public Object{
	public:
	Array<c_iContentObject* > m__data;
	int m__length;
	int m__cache;
	c_iContentManager();
	void p_SystemInit();
	c_iContentManager* m_new();
	void p_Expand();
	c_iContentObject* p_Get2(String);
	c_Image* p_GetImage(String,int,int);
	c_Image* p_GetImage2(String,int,int,int,int);
	Array<c_Image* > p_GetImage3(int,int,String,int,int);
	Array<c_Image* > p_GetImage4(int,int,String,int,int,int,int);
	void mark();
	String debug();
};
String dbg_type(c_iContentManager**p){return "iContentManager";}
class c_iContentObject : public Object{
	public:
	String m__path;
	c_Image* m__imagePointer;
	Array<c_Image* > m__imagePointers;
	c_iContentObject();
	c_iContentObject* m_new();
	void mark();
	String debug();
};
String dbg_type(c_iContentObject**p){return "iContentObject";}
extern c_iContentManager* bb_contentmanager_iContent;
class c_Player : public Object{
	public:
	c_Image* m_image;
	int m_x;
	int m_y;
	int m_xVel;
	int m_yVel;
	c_Animation* m_downAnim;
	c_Animation* m_currentAnimation;
	c_Player();
	c_Player* m_new(c_Image*,int,int);
	c_Player* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Player**p){return "Player";}
class c_Animation : public Object{
	public:
	c_Image* m_img;
	int m_frames;
	int m_width;
	int m_height;
	int m_frameTime;
	int m_elapsed;
	int m_frame;
	int m_lastTime;
	c_Animation();
	c_Animation* m_new(c_Image*,int,int,int,int);
	c_Animation* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Animation**p){return "Animation";}
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
class c_Level : public Object{
	public:
	Array<Array<int > > m_layout;
	bool m_generated;
	int m_xCoord;
	int m_yCoord;
	int m_width;
	int m_height;
	Array<c_Point* > m_walkways;
	Array<c_Point* > m_lava;
	int m_counter;
	int m_entranceX;
	int m_entranceY;
	int m_treasureX;
	int m_treasureY;
	c_Stack4* m_treasures;
	c_Level();
	int p_Draw(int,int);
	Array<Array<int > > p_setArray(int,int);
	int p_randomlyAssignCells(Array<Array<int > >);
	int p_checkWalls(Array<Array<int > >,int,int);
	Array<Array<int > > p_generateCellularly(Array<Array<int > >);
	int p_smoothEdges();
	int p_fillCells(Array<Array<int > >);
	int p_drunkWalk(Array<Array<int > >);
	int p_countWalkways();
	int p_makeLavaRiver(int,int);
	int p_makeLavaRivers();
	int p_addSpikesAndTraps();
	int p_makeStartingPoint();
	int p_setTreasure();
	bool p_setTreasure2(int);
	c_Level* m_new(int,int,int,int,String);
	c_Level* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Level**p){return "Level";}
extern c_Image* bb_level_caveTextures;
Float bb_random_Rnd();
Float bb_random_Rnd2(Float,Float);
Float bb_random_Rnd3(Float);
class c_Point : public Object{
	public:
	int m_x;
	int m_y;
	c_Point();
	c_Point* m_new(int,int);
	c_Point* m_new2();
	int p_getX();
	int p_getY();
	void mark();
	String debug();
};
String dbg_type(c_Point**p){return "Point";}
class c_AStarSearch : public Object{
	public:
	Array<Array<int > > m_gameMap;
	int m_maxDistance;
	bool m_allowDiagonal;
	c_Stack2* m_visited;
	c_Stack2* m_unvisited;
	c_Stack3* m_nodes;
	c_AStarSearch();
	c_AStarSearch* m_new(Array<Array<int > >,int,bool);
	c_AStarSearch* m_new2();
	int p_isWalkableTile(Array<Array<int > >,int,int);
	c_AStarNode* p_getLowestCostNode(c_Stack2*);
	c_AStarNode* p_getLowestCostNode2();
	bool p_remIfStackHasElement(c_Stack2*,c_AStarNode*);
	c_Path* p_findPath(int,int,int,int);
	bool p_isWalkableTileOverworld(Array<Array<int > >,int,int);
	int p_getTileCostOverworld(Array<Array<int > >,int,int);
	c_Path* p_findPathOverworld(Array<Array<int > >,int,int,int,int);
	void mark();
	String debug();
};
String dbg_type(c_AStarSearch**p){return "AStarSearch";}
class c_AStarNode : public Object{
	public:
	int m_x;
	int m_y;
	int m_id;
	Float m_cost;
	Float m_totalCost;
	Float m_heuristic;
	int m_depth;
	c_AStarNode* m_parent;
	c_AStarNode();
	c_AStarNode* m_new(int,int,int);
	c_AStarNode* m_new2();
	int p_setCost(int,int,int,int);
	int p_setParent(c_AStarNode*);
	void mark();
	String debug();
};
String dbg_type(c_AStarNode**p){return "AStarNode";}
class c_Stack2 : public Object{
	public:
	Array<c_AStarNode* > m_data;
	int m_length;
	c_Stack2();
	c_Stack2* m_new();
	c_Stack2* m_new2(Array<c_AStarNode* >);
	void p_Push4(c_AStarNode*);
	void p_Push5(Array<c_AStarNode* >,int,int);
	void p_Push6(Array<c_AStarNode* >,int);
	static c_AStarNode* m_NIL;
	void p_Length2(int);
	int p_Length();
	c_AStarNode* p_Get(int);
	void p_Clear();
	void p_Remove7(int);
	void mark();
	String debug();
};
String dbg_type(c_Stack2**p){return "Stack";}
class c_Stack3 : public Object{
	public:
	Array<c_Stack2* > m_data;
	int m_length;
	c_Stack3();
	c_Stack3* m_new();
	c_Stack3* m_new2(Array<c_Stack2* >);
	void p_Push7(c_Stack2*);
	void p_Push8(Array<c_Stack2* >,int,int);
	void p_Push9(Array<c_Stack2* >,int);
	c_Stack2* p_Get(int);
	static c_Stack2* m_NIL;
	void p_Length2(int);
	int p_Length();
	void mark();
	String debug();
};
String dbg_type(c_Stack3**p){return "Stack";}
class c_Stack4 : public Object{
	public:
	int m_length;
	Array<c_Point* > m_data;
	c_Stack4();
	void p_Push10(c_Point*);
	void p_Push11(Array<c_Point* >,int,int);
	void p_Push12(Array<c_Point* >,int);
	static c_Point* m_NIL;
	void p_Length2(int);
	int p_Length();
	c_Point* p_Get(int);
	c_Point* p_Pop();
	void mark();
	String debug();
};
String dbg_type(c_Stack4**p){return "Stack";}
int bb_math2_Max(int,int);
Float bb_math2_Max2(Float,Float);
Float bb_level_getDist(int,int,int,int);
Float bb_pathfinder_getCost(int,int,int,int);
bool bb_pathfinder_stackHasElement(c_Stack2*,c_AStarNode*);
class c_BoundingRect : public Object{
	public:
	int m_x;
	int m_y;
	int m_width;
	int m_height;
	int m_top;
	int m_bottom;
	int m_left;
	int m_right;
	c_BoundingRect();
	c_BoundingRect* m_new(int,int,int,int);
	c_BoundingRect* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_BoundingRect**p){return "BoundingRect";}
int bb_input_KeyDown(int);
extern c_Image* bb_noisetestscene_textures;
extern c_Image* bb_noisetestscene_enemies;
class c_SimplexNoise : public Object{
	public:
	Array<Array<int > > m_grad3;
	Array<int > m_p;
	Array<int > m_perm;
	c_SimplexNoise();
	c_SimplexNoise* m_new();
	Array<Array<Float > > p_setArray(int,int);
	Float p_dot(Array<int >,Float,Float);
	Float p_makeNoise(Float,Float);
	Float p_makeOctavedNoise(int,Float,Float,int,int,int);
	Array<Array<Float > > p_makeIsland(Array<Array<Float > >,int,int);
	Array<Array<Float > > p_generateOctavedNoiseMap(int,int,int,Float,Float);
	void mark();
	String debug();
};
String dbg_type(c_SimplexNoise**p){return "SimplexNoise";}
Array<Array<int > > bb_noisetestscene_setArray(int,int);
int bb_noisetestscene_randomlyAssignCells(Array<Array<int > >,int);
class c_Enumerator : public Object{
	public:
	c_List* m__list;
	c_Node2* m__curr;
	c_Enumerator();
	c_Enumerator* m_new(c_List*);
	c_Enumerator* m_new2();
	bool p_HasNext();
	c_PathStep* p_NextObject();
	void mark();
	String debug();
};
String dbg_type(c_Enumerator**p){return "Enumerator";}
class c_iConfig : public Object{
	public:
	c_iConfig();
	void mark();
	String debug();
};
String dbg_type(c_iConfig**p){return "iConfig";}
class c_iVector2d : public Object{
	public:
	Float m__x;
	Float m__y;
	c_iVector2d();
	c_iVector2d* m_new(Float,Float);
	c_iVector2d* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_iVector2d**p){return "iVector2d";}
class c_iControlSet : public Object{
	public:
	c_iControlSet();
	void mark();
	String debug();
};
String dbg_type(c_iControlSet**p){return "iControlSet";}
void gc_mark( BBGame *p ){}
String dbg_type( BBGame **p ){ return "BBGame"; }
String dbg_value( BBGame **p ){ return dbg_ptr_value( *p ); }
c_App::c_App(){
}
c_App* c_App::m_new(){
	DBG_ENTER("App.new")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<152>");
	if((bb_app__app)!=0){
		DBG_BLOCK();
		bbError(String(L"App has already been created",28));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<153>");
	gc_assign(bb_app__app,this);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<154>");
	gc_assign(bb_app__delegate,(new c_GameDelegate)->m_new());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<155>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<177>");
	bb_app_EndApp();
	return 0;
}
int c_App::p_OnBack(){
	DBG_ENTER("App.OnBack")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<181>");
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
c_iApp::c_iApp(){
}
c_iApp* c_iApp::m_new(){
	DBG_ENTER("iApp.new")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<294>");
	c_App::m_new();
	return this;
}
int c_iApp::p_OnBack(){
	DBG_ENTER("iApp.OnBack")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<297>");
	bb_app2_iBack();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<298>");
	return 0;
}
int c_iApp::p_OnClose(){
	DBG_ENTER("iApp.OnClose")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<302>");
	bb_app2_iClose();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<303>");
	return 0;
}
int c_iApp::p_OnLoading(){
	DBG_ENTER("iApp.OnLoading")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<307>");
	bb_app2_iLoading();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<308>");
	return 0;
}
int c_iApp::p_OnRender(){
	DBG_ENTER("iApp.OnRender")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<312>");
	bb_app2_iRender();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<313>");
	return 0;
}
int c_iApp::p_OnResize(){
	DBG_ENTER("iApp.OnResize")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<317>");
	bb_app2_iResize();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<318>");
	return 0;
}
int c_iApp::p_OnResume(){
	DBG_ENTER("iApp.OnResume")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<322>");
	bb_app2_iResume();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<323>");
	return 0;
}
int c_iApp::p_OnSuspend(){
	DBG_ENTER("iApp.OnSuspend")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<327>");
	bb_app2_iSuspend();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<328>");
	return 0;
}
int c_iApp::p_OnUpdate(){
	DBG_ENTER("iApp.OnUpdate")
	c_iApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<332>");
	bb_app2_iUpdate();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<333>");
	return 0;
}
void c_iApp::mark(){
	c_App::mark();
}
String c_iApp::debug(){
	String t="(iApp)\n";
	t=c_App::debug()+t;
	return t;
}
c_Game::c_Game(){
}
c_Game* c_Game::m_new(){
	DBG_ENTER("Game.new")
	c_Game *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<24>");
	c_iApp::m_new();
	return this;
}
int c_Game::p_OnCreate(){
	DBG_ENTER("Game.OnCreate")
	c_Game *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<29>");
	bb_random_Seed=bb_app_Millisecs();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<37>");
	gc_assign(bb_main_menu,(new c_MenuScene)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<38>");
	gc_assign(bb_main_gameplay,(new c_GameplayScene)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<39>");
	gc_assign(bb_main_noiseTest,(new c_NoiseTestScene)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<40>");
	c_Path* t_path=(new c_Path)->m_new();
	DBG_LOCAL(t_path,"path")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<43>");
	bb_app2_iStart3((bb_main_menu),60);
	return 0;
}
void c_Game::mark(){
	c_iApp::mark();
}
String c_Game::debug(){
	String t="(Game)\n";
	t=c_iApp::debug()+t;
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<65>");
	return this;
}
void c_GameDelegate::StartGame(){
	DBG_ENTER("GameDelegate.StartGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<75>");
	gc_assign(m__graphics,(new gxtkGraphics));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<76>");
	bb_graphics_SetGraphicsDevice(m__graphics);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<77>");
	bb_graphics_SetFont(0,32);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<79>");
	gc_assign(m__audio,(new gxtkAudio));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<80>");
	bb_audio_SetAudioDevice(m__audio);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<82>");
	gc_assign(m__input,(new c_InputDevice)->m_new());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<83>");
	bb_input_SetInputDevice(m__input);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<85>");
	bb_app_ValidateDeviceWindow(false);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<87>");
	bb_app_EnumDisplayModes();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<89>");
	bb_app__app->p_OnCreate();
}
void c_GameDelegate::SuspendGame(){
	DBG_ENTER("GameDelegate.SuspendGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<93>");
	bb_app__app->p_OnSuspend();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<94>");
	m__audio->Suspend();
}
void c_GameDelegate::ResumeGame(){
	DBG_ENTER("GameDelegate.ResumeGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<98>");
	m__audio->Resume();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<99>");
	bb_app__app->p_OnResume();
}
void c_GameDelegate::UpdateGame(){
	DBG_ENTER("GameDelegate.UpdateGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<103>");
	bb_app_ValidateDeviceWindow(true);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<104>");
	m__input->p_BeginUpdate();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<105>");
	bb_app__app->p_OnUpdate();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<106>");
	m__input->p_EndUpdate();
}
void c_GameDelegate::RenderGame(){
	DBG_ENTER("GameDelegate.RenderGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<110>");
	bb_app_ValidateDeviceWindow(true);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<111>");
	int t_mode=m__graphics->BeginRender();
	DBG_LOCAL(t_mode,"mode")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<112>");
	if((t_mode)!=0){
		DBG_BLOCK();
		bb_graphics_BeginRender();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<113>");
	if(t_mode==2){
		DBG_BLOCK();
		bb_app__app->p_OnLoading();
	}else{
		DBG_BLOCK();
		bb_app__app->p_OnRender();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<114>");
	if((t_mode)!=0){
		DBG_BLOCK();
		bb_graphics_EndRender();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<115>");
	m__graphics->EndRender();
}
void c_GameDelegate::KeyEvent(int t_event,int t_data){
	DBG_ENTER("GameDelegate.KeyEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<119>");
	m__input->p_KeyEvent(t_event,t_data);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<120>");
	if(t_event!=1){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<121>");
	int t_1=t_data;
	DBG_LOCAL(t_1,"1")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<122>");
	if(t_1==432){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<123>");
		bb_app__app->p_OnClose();
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<124>");
		if(t_1==416){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<125>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<130>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<134>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<138>");
	m__input->p_MotionEvent(t_event,t_data,t_x,t_y,t_z);
}
void c_GameDelegate::DiscardGraphics(){
	DBG_ENTER("GameDelegate.DiscardGraphics")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<142>");
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
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<312>");
	c_Game* t_g=(new c_Game)->m_new();
	return 0;
}
gxtkGraphics* bb_graphics_device;
int bb_graphics_SetGraphicsDevice(gxtkGraphics* t_dev){
	DBG_ENTER("SetGraphicsDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<63>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<70>");
	return this;
}
int c_Image::p_SetHandle(Float t_tx,Float t_ty){
	DBG_ENTER("Image.SetHandle")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<114>");
	this->m_tx=t_tx;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<115>");
	this->m_ty=t_ty;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<116>");
	this->m_flags=this->m_flags&-2;
	return 0;
}
int c_Image::p_ApplyFlags(int t_iflags){
	DBG_ENTER("Image.ApplyFlags")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<197>");
	m_flags=t_iflags;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<199>");
	if((m_flags&2)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<200>");
		Array<c_Frame* > t_=m_frames;
		int t_2=0;
		while(t_2<t_.Length()){
			DBG_BLOCK();
			c_Frame* t_f=t_.At(t_2);
			t_2=t_2+1;
			DBG_LOCAL(t_f,"f")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<201>");
			t_f->m_x+=1;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<203>");
		m_width-=2;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<206>");
	if((m_flags&4)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<207>");
		Array<c_Frame* > t_3=m_frames;
		int t_4=0;
		while(t_4<t_3.Length()){
			DBG_BLOCK();
			c_Frame* t_f2=t_3.At(t_4);
			t_4=t_4+1;
			DBG_LOCAL(t_f2,"f")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<208>");
			t_f2->m_y+=1;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<210>");
		m_height-=2;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<213>");
	if((m_flags&1)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<214>");
		p_SetHandle(Float(m_width)/FLOAT(2.0),Float(m_height)/FLOAT(2.0));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<217>");
	if(m_frames.Length()==1 && m_frames.At(0)->m_x==0 && m_frames.At(0)->m_y==0 && m_width==m_surface->Width() && m_height==m_surface->Height()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<218>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<143>");
	if((m_surface)!=0){
		DBG_BLOCK();
		bbError(String(L"Image already initialized",25));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<144>");
	gc_assign(m_surface,t_surf);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<146>");
	m_width=m_surface->Width()/t_nframes;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<147>");
	m_height=m_surface->Height();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<149>");
	gc_assign(m_frames,Array<c_Frame* >(t_nframes));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<150>");
	for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<151>");
		gc_assign(m_frames.At(t_i),(new c_Frame)->m_new(t_i*m_width,0));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<154>");
	p_ApplyFlags(t_iflags);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<155>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<159>");
	if((m_surface)!=0){
		DBG_BLOCK();
		bbError(String(L"Image already initialized",25));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<160>");
	gc_assign(m_surface,t_surf);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<161>");
	gc_assign(m_source,t_src);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<163>");
	m_width=t_iwidth;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<164>");
	m_height=t_iheight;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<166>");
	gc_assign(m_frames,Array<c_Frame* >(t_nframes));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<168>");
	int t_ix=t_x;
	int t_iy=t_y;
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<170>");
	for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<171>");
		if(t_ix+m_width>t_srcw){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<172>");
			t_ix=0;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<173>");
			t_iy+=m_height;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<175>");
		if(t_ix+m_width>t_srcw || t_iy+m_height>t_srch){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<176>");
			bbError(String(L"Image frame outside surface",27));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<178>");
		gc_assign(m_frames.At(t_i),(new c_Frame)->m_new(t_ix+t_srcx,t_iy+t_srcy));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<179>");
		t_ix+=m_width;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<182>");
	p_ApplyFlags(t_iflags);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<183>");
	return this;
}
int c_Image::p_Width(){
	DBG_ENTER("Image.Width")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<81>");
	return m_width;
}
int c_Image::p_Height(){
	DBG_ENTER("Image.Height")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<85>");
	return m_height;
}
int c_Image::p_Frames(){
	DBG_ENTER("Image.Frames")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<93>");
	int t_=m_frames.Length();
	return t_;
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<29>");
	return this;
}
int c_GraphicsContext::p_Validate(){
	DBG_ENTER("GraphicsContext.Validate")
	c_GraphicsContext *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<40>");
	if((m_matDirty)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<41>");
		bb_graphics_renderDevice->SetMatrix(bb_graphics_context->m_ix,bb_graphics_context->m_iy,bb_graphics_context->m_jx,bb_graphics_context->m_jy,bb_graphics_context->m_tx,bb_graphics_context->m_ty);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<42>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<7>");
	int t_i=t_path.Find(String(L":/",2),0);
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<8>");
	if(t_i!=-1 && t_path.Find(String(L"/",1),0)==t_i+1){
		DBG_BLOCK();
		return t_path;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<9>");
	if(t_path.StartsWith(String(L"./",2)) || t_path.StartsWith(String(L"/",1))){
		DBG_BLOCK();
		return t_path;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<10>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<23>");
	this->m_x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<24>");
	this->m_y=t_y;
	return this;
}
c_Frame* c_Frame::m_new2(){
	DBG_ENTER("Frame.new")
	c_Frame *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<18>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<239>");
	gxtkSurface* t_surf=bb_graphics_device->LoadSurface(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_surf,"surf")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<240>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<244>");
	gxtkSurface* t_surf=bb_graphics_device->LoadSurface(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_surf,"surf")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<245>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<548>");
	if(!((t_font)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<549>");
		if(!((bb_graphics_context->m_defaultFont)!=0)){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<550>");
			gc_assign(bb_graphics_context->m_defaultFont,bb_graphics_LoadImage(String(L"mojo_font.png",13),96,2));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<552>");
		t_font=bb_graphics_context->m_defaultFont;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<553>");
		t_firstChar=32;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<555>");
	gc_assign(bb_graphics_context->m_font,t_font);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<556>");
	bb_graphics_context->m_firstChar=t_firstChar;
	return 0;
}
gxtkAudio* bb_audio_device;
int bb_audio_SetAudioDevice(gxtkAudio* t_dev){
	DBG_ENTER("SetAudioDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<22>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<26>");
	for(int t_i=0;t_i<4;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<27>");
		gc_assign(m__joyStates.At(t_i),(new c_JoyState)->m_new());
	}
	return this;
}
void c_InputDevice::p_PutKeyHit(int t_key){
	DBG_ENTER("InputDevice.PutKeyHit")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<237>");
	if(m__keyHitPut==m__keyHitQueue.Length()){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<238>");
	m__keyHit.At(t_key)+=1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<239>");
	m__keyHitQueue.At(m__keyHitPut)=t_key;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<240>");
	m__keyHitPut+=1;
}
void c_InputDevice::p_BeginUpdate(){
	DBG_ENTER("InputDevice.BeginUpdate")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<189>");
	for(int t_i=0;t_i<4;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<190>");
		c_JoyState* t_state=m__joyStates.At(t_i);
		DBG_LOCAL(t_state,"state")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<191>");
		if(!BBGame::Game()->PollJoystick(t_i,t_state->m_joyx,t_state->m_joyy,t_state->m_joyz,t_state->m_buttons)){
			DBG_BLOCK();
			break;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<192>");
		for(int t_j=0;t_j<32;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<193>");
			int t_key=256+t_i*32+t_j;
			DBG_LOCAL(t_key,"key")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<194>");
			if(t_state->m_buttons.At(t_j)){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<195>");
				if(!m__keyDown.At(t_key)){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<196>");
					m__keyDown.At(t_key)=true;
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<197>");
					p_PutKeyHit(t_key);
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<200>");
				m__keyDown.At(t_key)=false;
			}
		}
	}
}
void c_InputDevice::p_EndUpdate(){
	DBG_ENTER("InputDevice.EndUpdate")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<207>");
	for(int t_i=0;t_i<m__keyHitPut;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<208>");
		m__keyHit.At(m__keyHitQueue.At(t_i))=0;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<210>");
	m__keyHitPut=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<211>");
	m__charGet=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<212>");
	m__charPut=0;
}
void c_InputDevice::p_KeyEvent(int t_event,int t_data){
	DBG_ENTER("InputDevice.KeyEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<111>");
	int t_1=t_event;
	DBG_LOCAL(t_1,"1")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<112>");
	if(t_1==1){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<113>");
		if(!m__keyDown.At(t_data)){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<114>");
			m__keyDown.At(t_data)=true;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<115>");
			p_PutKeyHit(t_data);
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<116>");
			if(t_data==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<117>");
				m__keyDown.At(384)=true;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<118>");
				p_PutKeyHit(384);
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<119>");
				if(t_data==384){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<120>");
					m__keyDown.At(1)=true;
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<121>");
					p_PutKeyHit(1);
				}
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<124>");
		if(t_1==2){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<125>");
			if(m__keyDown.At(t_data)){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<126>");
				m__keyDown.At(t_data)=false;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<127>");
				if(t_data==1){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<128>");
					m__keyDown.At(384)=false;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<129>");
					if(t_data==384){
						DBG_BLOCK();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<130>");
						m__keyDown.At(1)=false;
					}
				}
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<133>");
			if(t_1==3){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<134>");
				if(m__charPut<m__charQueue.Length()){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<135>");
					m__charQueue.At(m__charPut)=t_data;
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<136>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<142>");
	int t_2=t_event;
	DBG_LOCAL(t_2,"2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<143>");
	if(t_2==4){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<144>");
		p_KeyEvent(1,1+t_data);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<145>");
		if(t_2==5){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<146>");
			p_KeyEvent(2,1+t_data);
			return;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<148>");
			if(t_2==6){
				DBG_BLOCK();
			}else{
				DBG_BLOCK();
				return;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<152>");
	m__mouseX=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<153>");
	m__mouseY=t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<154>");
	m__touchX.At(0)=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<155>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<159>");
	int t_3=t_event;
	DBG_LOCAL(t_3,"3")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<160>");
	if(t_3==7){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<161>");
		p_KeyEvent(1,384+t_data);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<162>");
		if(t_3==8){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<163>");
			p_KeyEvent(2,384+t_data);
			return;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<165>");
			if(t_3==9){
				DBG_BLOCK();
			}else{
				DBG_BLOCK();
				return;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<169>");
	m__touchX.At(t_data)=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<170>");
	m__touchY.At(t_data)=t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<171>");
	if(t_data==0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<172>");
		m__mouseX=t_x;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<173>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<178>");
	int t_4=t_event;
	DBG_LOCAL(t_4,"4")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<179>");
	if(t_4==10){
		DBG_BLOCK();
	}else{
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<183>");
	m__accelX=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<184>");
	m__accelY=t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<185>");
	m__accelZ=t_z;
}
void c_InputDevice::p_Reset(){
	DBG_ENTER("InputDevice.Reset")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<32>");
	for(int t_i=0;t_i<512;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<33>");
		m__keyDown.At(t_i)=false;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<34>");
		m__keyHit.At(t_i)=0;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<36>");
	m__keyHitPut=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<37>");
	m__charGet=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<38>");
	m__charPut=0;
}
bool c_InputDevice::p_KeyDown(int t_key){
	DBG_ENTER("InputDevice.KeyDown")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<47>");
	if(t_key>0 && t_key<512){
		DBG_BLOCK();
		return m__keyDown.At(t_key);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<48>");
	return false;
}
Float c_InputDevice::p_TouchX(int t_index){
	DBG_ENTER("InputDevice.TouchX")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<77>");
	if(t_index>=0 && t_index<32){
		DBG_BLOCK();
		return m__touchX.At(t_index);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<78>");
	return FLOAT(0.0);
}
Float c_InputDevice::p_TouchY(int t_index){
	DBG_ENTER("InputDevice.TouchY")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<82>");
	if(t_index>=0 && t_index<32){
		DBG_BLOCK();
		return m__touchY.At(t_index);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<83>");
	return FLOAT(0.0);
}
int c_InputDevice::p_KeyHit(int t_key){
	DBG_ENTER("InputDevice.KeyHit")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<52>");
	if(t_key>0 && t_key<512){
		DBG_BLOCK();
		return m__keyHit.At(t_key);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<53>");
	return 0;
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<14>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<22>");
	gc_assign(bb_input_device,t_dev);
	return 0;
}
int bb_app__devWidth;
int bb_app__devHeight;
void bb_app_ValidateDeviceWindow(bool t_notifyApp){
	DBG_ENTER("ValidateDeviceWindow")
	DBG_LOCAL(t_notifyApp,"notifyApp")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<57>");
	int t_w=bb_app__game->GetDeviceWidth();
	DBG_LOCAL(t_w,"w")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<58>");
	int t_h=bb_app__game->GetDeviceHeight();
	DBG_LOCAL(t_h,"h")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<59>");
	if(t_w==bb_app__devWidth && t_h==bb_app__devHeight){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<60>");
	bb_app__devWidth=t_w;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<61>");
	bb_app__devHeight=t_h;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<62>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<192>");
	m__width=t_width;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<193>");
	m__height=t_height;
	return this;
}
c_DisplayMode* c_DisplayMode::m_new2(){
	DBG_ENTER("DisplayMode.new")
	c_DisplayMode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<189>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<7>");
	return this;
}
c_Node* c_Map::p_FindNode(int t_key){
	DBG_ENTER("Map.FindNode")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<157>");
	c_Node* t_node=m_root;
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<159>");
	while((t_node)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<160>");
		int t_cmp=p_Compare(t_key,t_node->m_key);
		DBG_LOCAL(t_cmp,"cmp")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<161>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<162>");
			t_node=t_node->m_right;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<163>");
			if(t_cmp<0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<164>");
				t_node=t_node->m_left;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<166>");
				return t_node;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<169>");
	return t_node;
}
bool c_Map::p_Contains(int t_key){
	DBG_ENTER("Map.Contains")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<25>");
	bool t_=p_FindNode(t_key)!=0;
	return t_;
}
int c_Map::p_RotateLeft(c_Node* t_node){
	DBG_ENTER("Map.RotateLeft")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<251>");
	c_Node* t_child=t_node->m_right;
	DBG_LOCAL(t_child,"child")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<252>");
	gc_assign(t_node->m_right,t_child->m_left);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<253>");
	if((t_child->m_left)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<254>");
		gc_assign(t_child->m_left->m_parent,t_node);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<256>");
	gc_assign(t_child->m_parent,t_node->m_parent);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<257>");
	if((t_node->m_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<258>");
		if(t_node==t_node->m_parent->m_left){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<259>");
			gc_assign(t_node->m_parent->m_left,t_child);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<261>");
			gc_assign(t_node->m_parent->m_right,t_child);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<264>");
		gc_assign(m_root,t_child);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<266>");
	gc_assign(t_child->m_left,t_node);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<267>");
	gc_assign(t_node->m_parent,t_child);
	return 0;
}
int c_Map::p_RotateRight(c_Node* t_node){
	DBG_ENTER("Map.RotateRight")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<271>");
	c_Node* t_child=t_node->m_left;
	DBG_LOCAL(t_child,"child")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<272>");
	gc_assign(t_node->m_left,t_child->m_right);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<273>");
	if((t_child->m_right)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<274>");
		gc_assign(t_child->m_right->m_parent,t_node);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<276>");
	gc_assign(t_child->m_parent,t_node->m_parent);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<277>");
	if((t_node->m_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<278>");
		if(t_node==t_node->m_parent->m_right){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<279>");
			gc_assign(t_node->m_parent->m_right,t_child);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<281>");
			gc_assign(t_node->m_parent->m_left,t_child);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<284>");
		gc_assign(m_root,t_child);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<286>");
	gc_assign(t_child->m_right,t_node);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<287>");
	gc_assign(t_node->m_parent,t_child);
	return 0;
}
int c_Map::p_InsertFixup(c_Node* t_node){
	DBG_ENTER("Map.InsertFixup")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<212>");
	while(((t_node->m_parent)!=0) && t_node->m_parent->m_color==-1 && ((t_node->m_parent->m_parent)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<213>");
		if(t_node->m_parent==t_node->m_parent->m_parent->m_left){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<214>");
			c_Node* t_uncle=t_node->m_parent->m_parent->m_right;
			DBG_LOCAL(t_uncle,"uncle")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<215>");
			if(((t_uncle)!=0) && t_uncle->m_color==-1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<216>");
				t_node->m_parent->m_color=1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<217>");
				t_uncle->m_color=1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<218>");
				t_uncle->m_parent->m_color=-1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<219>");
				t_node=t_uncle->m_parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<221>");
				if(t_node==t_node->m_parent->m_right){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<222>");
					t_node=t_node->m_parent;
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<223>");
					p_RotateLeft(t_node);
				}
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<225>");
				t_node->m_parent->m_color=1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<226>");
				t_node->m_parent->m_parent->m_color=-1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<227>");
				p_RotateRight(t_node->m_parent->m_parent);
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<230>");
			c_Node* t_uncle2=t_node->m_parent->m_parent->m_left;
			DBG_LOCAL(t_uncle2,"uncle")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<231>");
			if(((t_uncle2)!=0) && t_uncle2->m_color==-1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<232>");
				t_node->m_parent->m_color=1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<233>");
				t_uncle2->m_color=1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<234>");
				t_uncle2->m_parent->m_color=-1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<235>");
				t_node=t_uncle2->m_parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<237>");
				if(t_node==t_node->m_parent->m_left){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<238>");
					t_node=t_node->m_parent;
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<239>");
					p_RotateRight(t_node);
				}
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<241>");
				t_node->m_parent->m_color=1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<242>");
				t_node->m_parent->m_parent->m_color=-1;
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<243>");
				p_RotateLeft(t_node->m_parent->m_parent);
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<247>");
	m_root->m_color=1;
	return 0;
}
bool c_Map::p_Set(int t_key,c_DisplayMode* t_value){
	DBG_ENTER("Map.Set")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<29>");
	c_Node* t_node=m_root;
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<30>");
	c_Node* t_parent=0;
	int t_cmp=0;
	DBG_LOCAL(t_parent,"parent")
	DBG_LOCAL(t_cmp,"cmp")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<32>");
	while((t_node)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<33>");
		t_parent=t_node;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<34>");
		t_cmp=p_Compare(t_key,t_node->m_key);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<35>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<36>");
			t_node=t_node->m_right;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<37>");
			if(t_cmp<0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<38>");
				t_node=t_node->m_left;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<40>");
				gc_assign(t_node->m_value,t_value);
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<41>");
				return false;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<45>");
	t_node=(new c_Node)->m_new(t_key,t_value,-1,t_parent);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<47>");
	if((t_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<48>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<49>");
			gc_assign(t_parent->m_right,t_node);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<51>");
			gc_assign(t_parent->m_left,t_node);
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<53>");
		p_InsertFixup(t_node);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<55>");
		gc_assign(m_root,t_node);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<57>");
	return true;
}
bool c_Map::p_Insert(int t_key,c_DisplayMode* t_value){
	DBG_ENTER("Map.Insert")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<146>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<534>");
	c_Map::m_new();
	return this;
}
int c_IntMap::p_Compare(int t_lhs,int t_rhs){
	DBG_ENTER("IntMap.Compare")
	c_IntMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_lhs,"lhs")
	DBG_LOCAL(t_rhs,"rhs")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<537>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<13>");
	gc_assign(this->m_data,t_data.Slice(0));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<14>");
	this->m_length=t_data.Length();
	return this;
}
void c_Stack::p_Push(c_DisplayMode* t_value){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<71>");
	if(m_length==m_data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<72>");
		gc_assign(m_data,m_data.Resize(m_length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<74>");
	gc_assign(m_data.At(m_length),t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<75>");
	m_length+=1;
}
void c_Stack::p_Push2(Array<c_DisplayMode* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<83>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<84>");
		p_Push(t_values.At(t_offset+t_i));
	}
}
void c_Stack::p_Push3(Array<c_DisplayMode* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<79>");
	p_Push2(t_values,t_offset,t_values.Length()-t_offset);
}
Array<c_DisplayMode* > c_Stack::p_ToArray(){
	DBG_ENTER("Stack.ToArray")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<18>");
	Array<c_DisplayMode* > t_t=Array<c_DisplayMode* >(m_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<m_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<20>");
		gc_assign(t_t.At(t_i),m_data.At(t_i));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<22>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<364>");
	this->m_key=t_key;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<365>");
	gc_assign(this->m_value,t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<366>");
	this->m_color=t_color;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<367>");
	gc_assign(this->m_parent,t_parent);
	return this;
}
c_Node* c_Node::m_new2(){
	DBG_ENTER("Node.new")
	c_Node *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<361>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<263>");
	return bb_app__devWidth;
}
int bb_app_DeviceHeight(){
	DBG_ENTER("DeviceHeight")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<267>");
	return bb_app__devHeight;
}
void bb_app_EnumDisplayModes(){
	DBG_ENTER("EnumDisplayModes")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<33>");
	Array<BBDisplayMode* > t_modes=bb_app__game->GetDisplayModes();
	DBG_LOCAL(t_modes,"modes")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<34>");
	c_IntMap* t_mmap=(new c_IntMap)->m_new();
	DBG_LOCAL(t_mmap,"mmap")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<35>");
	c_Stack* t_mstack=(new c_Stack)->m_new();
	DBG_LOCAL(t_mstack,"mstack")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<36>");
	for(int t_i=0;t_i<t_modes.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<37>");
		int t_w=t_modes.At(t_i)->width;
		DBG_LOCAL(t_w,"w")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<38>");
		int t_h=t_modes.At(t_i)->height;
		DBG_LOCAL(t_h,"h")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<39>");
		int t_size=t_w<<16|t_h;
		DBG_LOCAL(t_size,"size")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<40>");
		if(t_mmap->p_Contains(t_size)){
			DBG_BLOCK();
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<42>");
			c_DisplayMode* t_mode=(new c_DisplayMode)->m_new(t_modes.At(t_i)->width,t_modes.At(t_i)->height);
			DBG_LOCAL(t_mode,"mode")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<43>");
			t_mmap->p_Insert(t_size,t_mode);
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<44>");
			t_mstack->p_Push(t_mode);
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<47>");
	gc_assign(bb_app__displayModes,t_mstack->p_ToArray());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<48>");
	BBDisplayMode* t_mode2=bb_app__game->GetDesktopMode();
	DBG_LOCAL(t_mode2,"mode")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<49>");
	if((t_mode2)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<50>");
		gc_assign(bb_app__desktopMode,(new c_DisplayMode)->m_new(t_mode2->width,t_mode2->height));
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<52>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<312>");
	bb_graphics_context->m_ix=t_ix;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<313>");
	bb_graphics_context->m_iy=t_iy;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<314>");
	bb_graphics_context->m_jx=t_jx;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<315>");
	bb_graphics_context->m_jy=t_jy;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<316>");
	bb_graphics_context->m_tx=t_tx;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<317>");
	bb_graphics_context->m_ty=t_ty;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<318>");
	bb_graphics_context->m_tformed=((t_ix!=FLOAT(1.0) || t_iy!=FLOAT(0.0) || t_jx!=FLOAT(0.0) || t_jy!=FLOAT(1.0) || t_tx!=FLOAT(0.0) || t_ty!=FLOAT(0.0))?1:0);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<319>");
	bb_graphics_context->m_matDirty=1;
	return 0;
}
int bb_graphics_SetMatrix2(Array<Float > t_m){
	DBG_ENTER("SetMatrix")
	DBG_LOCAL(t_m,"m")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<308>");
	bb_graphics_SetMatrix(t_m.At(0),t_m.At(1),t_m.At(2),t_m.At(3),t_m.At(4),t_m.At(5));
	return 0;
}
int bb_graphics_SetColor(Float t_r,Float t_g,Float t_b){
	DBG_ENTER("SetColor")
	DBG_LOCAL(t_r,"r")
	DBG_LOCAL(t_g,"g")
	DBG_LOCAL(t_b,"b")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<254>");
	bb_graphics_context->m_color_r=t_r;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<255>");
	bb_graphics_context->m_color_g=t_g;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<256>");
	bb_graphics_context->m_color_b=t_b;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<257>");
	bb_graphics_renderDevice->SetColor(t_r,t_g,t_b);
	return 0;
}
int bb_graphics_SetAlpha(Float t_alpha){
	DBG_ENTER("SetAlpha")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<271>");
	bb_graphics_context->m_alpha=t_alpha;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<272>");
	bb_graphics_renderDevice->SetAlpha(t_alpha);
	return 0;
}
int bb_graphics_SetBlend(int t_blend){
	DBG_ENTER("SetBlend")
	DBG_LOCAL(t_blend,"blend")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<280>");
	bb_graphics_context->m_blend=t_blend;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<281>");
	bb_graphics_renderDevice->SetBlend(t_blend);
	return 0;
}
int bb_graphics_SetScissor(Float t_x,Float t_y,Float t_width,Float t_height){
	DBG_ENTER("SetScissor")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<289>");
	bb_graphics_context->m_scissor_x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<290>");
	bb_graphics_context->m_scissor_y=t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<291>");
	bb_graphics_context->m_scissor_width=t_width;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<292>");
	bb_graphics_context->m_scissor_height=t_height;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<293>");
	bb_graphics_renderDevice->SetScissor(int(t_x),int(t_y),int(t_width),int(t_height));
	return 0;
}
int bb_graphics_BeginRender(){
	DBG_ENTER("BeginRender")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<225>");
	gc_assign(bb_graphics_renderDevice,bb_graphics_device);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<226>");
	bb_graphics_context->m_matrixSp=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<227>");
	bb_graphics_SetMatrix(FLOAT(1.0),FLOAT(0.0),FLOAT(0.0),FLOAT(1.0),FLOAT(0.0),FLOAT(0.0));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<228>");
	bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<229>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<230>");
	bb_graphics_SetBlend(0);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<231>");
	bb_graphics_SetScissor(FLOAT(0.0),FLOAT(0.0),Float(bb_app_DeviceWidth()),Float(bb_app_DeviceHeight()));
	return 0;
}
int bb_graphics_EndRender(){
	DBG_ENTER("EndRender")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<235>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<259>");
	bbError(String());
}
c_iObject::c_iObject(){
	m__enabled=1;
	m__tattoo=false;
	m__inPool=0;
}
int c_iObject::p_OnDisabled(){
	DBG_ENTER("iObject.OnDisabled")
	c_iObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<153>");
	return 0;
}
c_iObject* c_iObject::m_new(){
	DBG_ENTER("iObject.new")
	c_iObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<20>");
	return this;
}
int c_iObject::p_OnDisable(){
	DBG_ENTER("iObject.OnDisable")
	c_iObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<146>");
	return 0;
}
void c_iObject::p_Disable(){
	DBG_ENTER("iObject.Disable")
	c_iObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<69>");
	m__enabled=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<71>");
	p_OnDisable();
}
void c_iObject::mark(){
	Object::mark();
}
String c_iObject::debug(){
	String t="(iObject)\n";
	t+=dbg_decl("_enabled",&m__enabled);
	t+=dbg_decl("_inPool",&m__inPool);
	t+=dbg_decl("_tattoo",&m__tattoo);
	return t;
}
c_iScene::c_iScene(){
	m__visible=1;
	m__started=false;
	m__cold=true;
	m__paused=false;
}
int c_iScene::p_OnBack(){
	DBG_ENTER("iScene.OnBack")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<402>");
	return 0;
}
int c_iScene::p_OnClose(){
	DBG_ENTER("iScene.OnClose")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<415>");
	return 0;
}
int c_iScene::p_OnLoading(){
	DBG_ENTER("iScene.OnLoading")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<440>");
	return 0;
}
int c_iScene::p_OnRender(){
	DBG_ENTER("iScene.OnRender")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<472>");
	return 0;
}
void c_iScene::p_Render(){
	DBG_ENTER("iScene.Render")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<540>");
	p_OnRender();
}
int c_iScene::p_OnResize(){
	DBG_ENTER("iScene.OnResize")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<483>");
	return 0;
}
void c_iScene::p_Resume(){
	DBG_ENTER("iScene.Resume")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
}
int c_iScene::p_OnResume(){
	DBG_ENTER("iScene.OnResume")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<489>");
	return 0;
}
void c_iScene::p_Suspend(){
	DBG_ENTER("iScene.Suspend")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
}
int c_iScene::p_OnSuspend(){
	DBG_ENTER("iScene.OnSuspend")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<514>");
	return 0;
}
int c_iScene::p_OnStop(){
	DBG_ENTER("iScene.OnStop")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<503>");
	return 0;
}
void c_iScene::p_Set2(){
	DBG_ENTER("iScene.Set")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<569>");
	gc_assign(bb_app2_iCurrentScene,this);
}
int c_iScene::p_OnColdStart(){
	DBG_ENTER("iScene.OnColdStart")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<424>");
	return 0;
}
int c_iScene::p_OnStart(){
	DBG_ENTER("iScene.OnStart")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<496>");
	return 0;
}
int c_iScene::p_OnPaused(){
	DBG_ENTER("iScene.OnPaused")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<456>");
	return 0;
}
int c_iScene::p_OnUpdate(){
	DBG_ENTER("iScene.OnUpdate")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<521>");
	return 0;
}
void c_iScene::p_Update(){
	DBG_ENTER("iScene.Update")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<598>");
	if((bb_app2_iNextScene)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<600>");
		if(m__started){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<601>");
			bb_input_ResetInput();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<602>");
			m__started=false;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<603>");
			p_OnStop();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<606>");
		bb_app2_iNextScene->p_Set2();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<608>");
		bb_app2_iNextScene=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<610>");
		if(bb_app2_iCurrentScene->m__cold==true){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<611>");
			bb_app2_iCurrentScene->p_OnColdStart();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<612>");
			bb_app2_iCurrentScene->m__cold=false;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<615>");
		if(bb_app2_iCurrentScene->m__started==false){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<616>");
			bb_app2_iCurrentScene->p_OnStart();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<617>");
			bb_app2_iCurrentScene->m__started=true;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<620>");
		if(bb_app2_iCurrentScene->m__enabled==1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<622>");
			if(bb_app2_iCurrentScene->m__paused){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<623>");
				bb_app2_iCurrentScene->p_OnPaused();
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<625>");
				bb_app2_iCurrentScene->p_OnUpdate();
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<630>");
			if(bb_app2_iCurrentScene->m__enabled>1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<631>");
				bb_app2_iCurrentScene->p_OnDisabled();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<632>");
				bb_app2_iCurrentScene->m__enabled=bb_app2_iCurrentScene->m__enabled-1;
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<639>");
		if(bb_app2_iCurrentScene->m__cold==true){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<640>");
			bb_app2_iCurrentScene->p_OnColdStart();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<641>");
			bb_app2_iCurrentScene->m__cold=false;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<644>");
		if(bb_app2_iCurrentScene->m__started==false){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<645>");
			bb_app2_iCurrentScene->p_OnStart();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<646>");
			bb_app2_iCurrentScene->m__started=true;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<649>");
		if(bb_app2_iCurrentScene->m__enabled==1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<651>");
			if(bb_app2_iCurrentScene->m__paused){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<652>");
				bb_app2_iCurrentScene->p_OnPaused();
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<654>");
				bb_app2_iCurrentScene->p_OnUpdate();
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<659>");
			if(bb_app2_iCurrentScene->m__enabled>1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<660>");
				bb_app2_iCurrentScene->p_OnDisabled();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<661>");
				bb_app2_iCurrentScene->m__enabled=bb_app2_iCurrentScene->m__enabled-1;
			}
		}
	}
}
int c_iScene::p_OnCreate(){
	DBG_ENTER("iScene.OnCreate")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<433>");
	return 0;
}
void c_iScene::p_SystemInit(){
	DBG_ENTER("iScene.SystemInit")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<587>");
	p_Set2();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<589>");
	p_OnCreate();
}
c_iScene* c_iScene::m_new(){
	DBG_ENTER("iScene.new")
	c_iScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<388>");
	c_iObject::m_new();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<389>");
	p_SystemInit();
	return this;
}
void c_iScene::mark(){
	c_iObject::mark();
}
String c_iScene::debug(){
	String t="(iScene)\n";
	t=c_iObject::debug()+t;
	t+=dbg_decl("_cold",&m__cold);
	t+=dbg_decl("_paused",&m__paused);
	t+=dbg_decl("_started",&m__started);
	t+=dbg_decl("_visible",&m__visible);
	return t;
}
c_iScene* bb_app2_iCurrentScene;
void bb_app2_iBack(){
	DBG_ENTER("iBack")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<46>");
	if(((bb_app2_iCurrentScene)!=0) && ((bb_app2_iCurrentScene->m__enabled)!=0)){
		DBG_BLOCK();
		bb_app2_iCurrentScene->p_OnBack();
	}
}
void bb_app2_iClose(){
	DBG_ENTER("iClose")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<64>");
	if(((bb_app2_iCurrentScene)!=0) && ((bb_app2_iCurrentScene->m__enabled)!=0)){
		DBG_BLOCK();
		bb_app2_iCurrentScene->p_OnClose();
	}
}
void bb_app2_iLoading(){
	DBG_ENTER("iLoading")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<99>");
	if(((bb_app2_iCurrentScene)!=0) && ((bb_app2_iCurrentScene->m__enabled)!=0)){
		DBG_BLOCK();
		bb_app2_iCurrentScene->p_OnLoading();
	}
}
void bb_app2_iRender(){
	DBG_ENTER("iRender")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<143>");
	if((bb_app2_iCurrentScene)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<145>");
		if((bb_app2_iCurrentScene->m__enabled)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<147>");
			if(bb_app2_iCurrentScene->m__visible==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<148>");
				bb_app2_iCurrentScene->p_Render();
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<150>");
				if(bb_app2_iCurrentScene->m__visible>1){
					DBG_BLOCK();
					bb_app2_iCurrentScene->m__visible=bb_app2_iCurrentScene->m__visible-1;
				}
			}
		}
	}
}
void bb_app2_iResize(){
	DBG_ENTER("iResize")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<185>");
	if(((bb_app2_iCurrentScene)!=0) && ((bb_app2_iCurrentScene->m__enabled)!=0)){
		DBG_BLOCK();
		bb_app2_iCurrentScene->p_OnResize();
	}
}
void bb_app2_iResume(){
	DBG_ENTER("iResume")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<203>");
	if(((bb_app2_iCurrentScene)!=0) && ((bb_app2_iCurrentScene->m__enabled)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<204>");
		bb_app2_iCurrentScene->p_Resume();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<205>");
		bb_app2_iCurrentScene->p_OnResume();
	}
}
void bb_app2_iSuspend(){
	DBG_ENTER("iSuspend")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<249>");
	if(((bb_app2_iCurrentScene)!=0) && ((bb_app2_iCurrentScene->m__enabled)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<250>");
		bb_app2_iCurrentScene->p_Suspend();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<251>");
		bb_app2_iCurrentScene->p_OnSuspend();
	}
}
c_iDeltaTimer::c_iDeltaTimer(){
	m__targetFPS=FLOAT(60.0);
	m__lastTicks=0;
	m__frameTime=FLOAT(.0);
	m__elapsedTime=FLOAT(0.0);
	m__timeScale=FLOAT(1.0);
	m__deltaTime=FLOAT(.0);
	m__elapsedDelta=FLOAT(0.0);
}
c_iDeltaTimer* c_iDeltaTimer::m_new(Float t_targetFPS){
	DBG_ENTER("iDeltaTimer.new")
	c_iDeltaTimer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_targetFPS,"targetFPS")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<24>");
	m__targetFPS=t_targetFPS;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<25>");
	m__lastTicks=bb_app_Millisecs();
	return this;
}
c_iDeltaTimer* c_iDeltaTimer::m_new2(){
	DBG_ENTER("iDeltaTimer.new")
	c_iDeltaTimer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<11>");
	return this;
}
void c_iDeltaTimer::p_Update(){
	DBG_ENTER("iDeltaTimer.Update")
	c_iDeltaTimer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<94>");
	m__frameTime=Float(bb_app_Millisecs()-m__lastTicks);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<95>");
	m__lastTicks=bb_app_Millisecs();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<96>");
	m__elapsedTime+=m__frameTime;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<97>");
	m__deltaTime=m__frameTime/(FLOAT(1000.0)/m__targetFPS)*m__timeScale;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<98>");
	m__elapsedDelta=m__elapsedDelta+m__deltaTime/m__targetFPS;
}
void c_iDeltaTimer::p_Resume(){
	DBG_ENTER("iDeltaTimer.Resume")
	c_iDeltaTimer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<57>");
	m__lastTicks=bb_app_Millisecs();
}
void c_iDeltaTimer::mark(){
	Object::mark();
}
String c_iDeltaTimer::debug(){
	String t="(iDeltaTimer)\n";
	t+=dbg_decl("_targetFPS",&m__targetFPS);
	t+=dbg_decl("_lastTicks",&m__lastTicks);
	t+=dbg_decl("_frameTime",&m__frameTime);
	t+=dbg_decl("_deltaTime",&m__deltaTime);
	t+=dbg_decl("_timeScale",&m__timeScale);
	t+=dbg_decl("_elapsedDelta",&m__elapsedDelta);
	t+=dbg_decl("_elapsedTime",&m__elapsedTime);
	return t;
}
int bb_app_Millisecs(){
	DBG_ENTER("Millisecs")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<233>");
	int t_=bb_app__game->Millisecs();
	return t_;
}
c_iDeltaTimer* bb_app2_iDT;
int bb_app2_iSpeed;
c_iScene* bb_app2_iNextScene;
int bb_input_ResetInput(){
	DBG_ENTER("ResetInput")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<26>");
	bb_input_device->p_Reset();
	return 0;
}
void bb_app2_iUpdate(){
	DBG_ENTER("iUpdate")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<271>");
	bb_app2_iDT->p_Update();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<273>");
	if((bb_app2_iCurrentScene)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<274>");
		if(bb_app2_iSpeed==1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<275>");
			bb_app2_iCurrentScene->p_Update();
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<277>");
			for(int t_i=0;t_i<bb_app2_iSpeed;t_i=t_i+1){
				DBG_BLOCK();
				DBG_LOCAL(t_i,"i")
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<278>");
				bb_app2_iCurrentScene->p_Update();
			}
		}
	}
}
int bb_random_Seed;
c_iEngine::c_iEngine(){
	m__timeCode=0;
	m__autoCls=true;
	m__clsRed=FLOAT(.0);
	m__colorFade=FLOAT(1.0);
	m__clsGreen=FLOAT(.0);
	m__clsBlue=FLOAT(.0);
	m__renderToPlayfield=true;
	m__playfieldList=(new c_iList)->m_new();
	m__currentPlayfield=0;
	m__alphaFade=FLOAT(1.0);
	m__borders=true;
	m__showSystemGui=false;
	m__taskList=(new c_iList5)->m_new();
	m__countDown=0;
	m__scoreCount=0;
	m__playfieldPointer=0;
	m__collisionReadList=(new c_iList3)->m_new();
	m__layerPointer=0;
	m__collisionWriteList=(new c_iList3)->m_new();
}
c_iEngine* c_iEngine::m_new(){
	DBG_ENTER("iEngine.new")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<13>");
	c_iScene::m_new();
	return this;
}
int c_iEngine::p_OnColdStart(){
	DBG_ENTER("iEngine.OnColdStart")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<381>");
	return 0;
}
int c_iEngine::p_OnResize(){
	DBG_ENTER("iEngine.OnResize")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<397>");
	return 0;
}
Float c_iEngine::p_AlphaFade(){
	DBG_ENTER("iEngine.AlphaFade")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<139>");
	return m__alphaFade;
}
void c_iEngine::p_AlphaFade2(Float t_alpha){
	DBG_ENTER("iEngine.AlphaFade")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<146>");
	m__alphaFade=t_alpha;
}
int c_iEngine::p_SetAlpha(Float t_alpha){
	DBG_ENTER("iEngine.SetAlpha")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<626>");
	if((m__currentPlayfield)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<627>");
		m__currentPlayfield->p_SetAlpha(t_alpha);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<629>");
		bb_graphics_SetAlpha(t_alpha*bb_globals_iEnginePointer->p_AlphaFade());
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<632>");
	return 0;
}
Float c_iEngine::p_ColorFade(){
	DBG_ENTER("iEngine.ColorFade")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<194>");
	return m__colorFade;
}
void c_iEngine::p_ColorFade2(Float t_value){
	DBG_ENTER("iEngine.ColorFade")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<201>");
	m__colorFade=t_value;
}
int c_iEngine::p_SetColor(Float t_red,Float t_green,Float t_blue){
	DBG_ENTER("iEngine.SetColor")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_red,"red")
	DBG_LOCAL(t_green,"green")
	DBG_LOCAL(t_blue,"blue")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<641>");
	if((m__currentPlayfield)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<642>");
		m__currentPlayfield->p_SetColor(t_red,t_green,t_blue);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<644>");
		bb_graphics_SetColor(t_red*bb_globals_iEnginePointer->p_ColorFade(),t_green*bb_globals_iEnginePointer->p_ColorFade(),t_blue*bb_globals_iEnginePointer->p_ColorFade());
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<647>");
	return 0;
}
int c_iEngine::p_OnTopRender(){
	DBG_ENTER("iEngine.OnTopRender")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<406>");
	return 0;
}
void c_iEngine::p_RenderPlayfields(){
	DBG_ENTER("iEngine.RenderPlayfields")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<563>");
	gc_assign(m__currentPlayfield,m__playfieldList->p_First());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<565>");
	while((m__currentPlayfield)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<567>");
		if(m__currentPlayfield->m__enabled==1 && m__currentPlayfield->m__visible){
			DBG_BLOCK();
			m__currentPlayfield->p_Render();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<569>");
		gc_assign(m__currentPlayfield,m__playfieldList->p_Ascend());
	}
}
void c_iEngine::p_RenderSystemGui(){
	DBG_ENTER("iEngine.RenderSystemGui")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<580>");
	if(m__showSystemGui && ((c_iSystemGui::m__playfield)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<582>");
		c_iStack2D::m_Push();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<584>");
		bb_graphics_Scale(c_iSystemGui::m__playfield->m__scaleX,c_iSystemGui::m__playfield->m__scaleY);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<586>");
		bb_graphics_Translate(c_iSystemGui::m__playfield->p_PositionX()/c_iSystemGui::m__playfield->m__scaleX,c_iSystemGui::m__playfield->p_PositionY()/c_iSystemGui::m__playfield->m__scaleY);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<588>");
		c_iGuiObject* t_g=c_iSystemGui::m__playfield->m__guiList.At(c_iSystemGui::m__playfield->m__guiPage)->p_First();
		DBG_LOCAL(t_g,"g")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<589>");
		while((t_g)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<590>");
			if(t_g->m__enabled==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<591>");
				if(t_g->m__ghost){
					DBG_BLOCK();
					t_g->p_RenderGhost();
				}else{
					DBG_BLOCK();
					t_g->p_Render();
				}
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<592>");
				t_g=c_iSystemGui::m__playfield->m__guiList.At(c_iSystemGui::m__playfield->m__guiPage)->p_Ascend();
			}
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<596>");
		c_iStack2D::m_Pop();
	}
}
int c_iEngine::p_DebugRender(){
	DBG_ENTER("iEngine.DebugRender")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<217>");
	return 0;
}
void c_iEngine::p_Render(){
	DBG_ENTER("iEngine.Render")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<501>");
	if((bb_functions_iTimeCode())!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<503>");
		if(m__autoCls){
			DBG_BLOCK();
			bb_gfx_iCls(m__clsRed*m__colorFade,m__clsGreen*m__colorFade,m__clsBlue*m__colorFade);
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<505>");
		if(m__renderToPlayfield==false || m__playfieldList->p_Length()==0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<507>");
			c_iStack2D::m_Push();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<509>");
			p_SetAlpha(FLOAT(1.0));
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<510>");
			p_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<512>");
			p_OnRender();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<514>");
			c_iStack2D::m_Pop();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<518>");
		p_RenderPlayfields();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<520>");
		if(m__renderToPlayfield==false || m__playfieldList->p_Length()==0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<522>");
			c_iStack2D::m_Push();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<524>");
			p_SetAlpha(FLOAT(1.0));
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<525>");
			p_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<527>");
			p_OnTopRender();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<529>");
			c_iStack2D::m_Pop();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<533>");
		p_RenderSystemGui();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<535>");
		c_iTask::m_Render(m__taskList);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<537>");
		c_iTask::m_Render(bb_globals_iTaskList);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<539>");
		c_iStack2D::m_Push();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<541>");
		Float t_a=m__alphaFade;
		DBG_LOCAL(t_a,"a")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<542>");
		Float t_c=m__colorFade;
		DBG_LOCAL(t_c,"c")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<544>");
		m__alphaFade=FLOAT(1.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<545>");
		m__colorFade=FLOAT(1.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<547>");
		p_DebugRender();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<549>");
		m__alphaFade=t_a;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<550>");
		m__colorFade=t_c;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<552>");
		c_iStack2D::m_Pop();
	}
}
void c_iEngine::p_Resume(){
	DBG_ENTER("iEngine.Resume")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<603>");
	bb_app2_iDT->p_Resume();
}
void c_iEngine::p_Set2(){
	DBG_ENTER("iEngine.Set")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<617>");
	gc_assign(bb_app2_iCurrentScene,(this));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<618>");
	gc_assign(bb_globals_iEnginePointer,this);
}
int c_iEngine::p_OnGuiPageChange(c_iPlayfield* t_playfield){
	DBG_ENTER("iEngine.OnGuiPageChange")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_playfield,"playfield")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<388>");
	return 0;
}
void c_iEngine::p_UpdateSystemGui(){
	DBG_ENTER("iEngine.UpdateSystemGui")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<867>");
	if(m__showSystemGui && ((c_iSystemGui::m__playfield)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<869>");
		if(c_iSystemGui::m__playfield->m__nextGuiPage>=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<870>");
			c_iSystemGui::m__playfield->m__guiPage=c_iSystemGui::m__playfield->m__nextGuiPage;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<871>");
			c_iSystemGui::m__playfield->m__nextGuiPage=-1;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<874>");
		c_iGuiObject* t_g=0;
		DBG_LOCAL(t_g,"g")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<876>");
		t_g=c_iSystemGui::m__playfield->m__guiList.At(c_iSystemGui::m__playfield->m__guiPage)->p_Last();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<877>");
		while((t_g)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<879>");
			t_g->p_UpdateWorldXY();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<881>");
			if(t_g->m__enabled==1 && !t_g->m__ghost){
				DBG_BLOCK();
				t_g->p_UpdateInput();
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<883>");
			t_g=c_iSystemGui::m__playfield->m__guiList.At(c_iSystemGui::m__playfield->m__guiPage)->p_Descend();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<887>");
		t_g=c_iSystemGui::m__playfield->m__guiList.At(c_iSystemGui::m__playfield->m__guiPage)->p_First();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<888>");
		while((t_g)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<890>");
			if(t_g->m__enabled==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<892>");
				if(t_g->m__ghost){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<893>");
					t_g->p_UpdateGhost();
				}else{
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<895>");
					t_g->p_Update();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<896>");
					t_g->p_OnUpdate();
				}
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<901>");
			t_g=c_iSystemGui::m__playfield->m__guiList.At(c_iSystemGui::m__playfield->m__guiPage)->p_Ascend();
		}
	}
}
void c_iEngine::p_UpdatePlayfields(){
	DBG_ENTER("iEngine.UpdatePlayfields")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<819>");
	gc_assign(m__currentPlayfield,m__playfieldList->p_First());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<821>");
	while((m__currentPlayfield)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<822>");
		if((m__currentPlayfield->m__enabled)!=0){
			DBG_BLOCK();
			m__currentPlayfield->p_Update();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<823>");
		gc_assign(m__currentPlayfield,m__playfieldList->p_Ascend());
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<826>");
	gc_assign(m__currentPlayfield,m__playfieldList->p_First());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<828>");
	while((m__currentPlayfield)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<830>");
		if((m__currentPlayfield->m__enabled)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<832>");
			gc_assign(m__currentPlayfield->m__currentLayer,m__currentPlayfield->m__layerList->p_First());
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<834>");
			while((m__currentPlayfield->m__currentLayer)!=0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<836>");
				if(((m__currentPlayfield->m__currentLayer->m__enabled)!=0) && m__currentPlayfield->m__currentLayer->m__lastUpdate!=m__timeCode){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<838>");
					m__currentPlayfield->m__currentLayer->m__lastUpdate=m__timeCode;
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<840>");
					m__currentPlayfield->m__currentLayer->p_Update();
				}
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<844>");
				gc_assign(m__currentPlayfield->m__currentLayer,m__currentPlayfield->m__layerList->p_Ascend());
			}
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<850>");
		gc_assign(m__currentPlayfield,m__playfieldList->p_Ascend());
	}
}
void c_iEngine::p_UpdateService(){
	DBG_ENTER("iEngine.UpdateService")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
}
void c_iEngine::p_UpdateCollisions(){
	DBG_ENTER("iEngine.UpdateCollisions")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<757>");
	c_iLayerObject* t_s1=m__collisionReadList->p_First();
	DBG_LOCAL(t_s1,"s1")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<758>");
	while((t_s1)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<760>");
		if(((t_s1->p_Layer())!=0) && t_s1->p_Layer()->m__enabled==1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<762>");
			c_iLayerObject* t_s2=m__collisionWriteList->p_First();
			DBG_LOCAL(t_s2,"s2")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<763>");
			while((t_s2)!=0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<765>");
				if(((t_s2->p_Layer())!=0) && t_s2->p_Layer()->m__enabled==1){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<766>");
					if((t_s1->p_Collides(t_s2))!=0){
						DBG_BLOCK();
						t_s1->p_OnCollision(t_s2);
					}
				}
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<769>");
				t_s2=m__collisionWriteList->p_Ascend();
			}
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<775>");
		t_s1=m__collisionReadList->p_Ascend();
	}
}
void c_iEngine::p_UpdateLogic(){
	DBG_ENTER("iEngine.UpdateLogic")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<786>");
	if(m__enabled==1){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<788>");
		p_UpdatePlayfields();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<790>");
		p_UpdateSystemGui();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<792>");
		p_UpdateService();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<794>");
		p_OnUpdate();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<796>");
		p_UpdateCollisions();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<798>");
		m__timeCode=m__timeCode+1;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<802>");
		if(m__enabled>1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<804>");
			p_OnDisabled();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<806>");
			m__enabled=m__enabled-1;
		}
	}
}
void c_iEngine::p_UpdateFixed(){
	DBG_ENTER("iEngine.UpdateFixed")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<914>");
	if(m__paused){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<916>");
		p_UpdateSystemGui();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<917>");
		p_OnPaused();
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<921>");
		p_UpdateLogic();
	}
}
void c_iEngine::p_Update(){
	DBG_ENTER("iEngine.Update")
	c_iEngine *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<688>");
	c_iGuiObject::m__topObject=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<690>");
	if((bb_app2_iNextScene)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<692>");
		if(m__started){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<693>");
			bb_input_ResetInput();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<694>");
			m__started=false;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<695>");
			p_OnStop();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<698>");
		bb_app2_iNextScene->p_Set2();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<700>");
		bb_app2_iNextScene=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<702>");
		bb_globals_iEnginePointer->m__alphaFade=FLOAT(1.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<703>");
		bb_globals_iEnginePointer->m__colorFade=FLOAT(1.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<704>");
		bb_globals_iEnginePointer->m__countDown=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<705>");
		bb_globals_iEnginePointer->m__paused=false;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<706>");
		bb_globals_iEnginePointer->m__scoreCount=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<707>");
		bb_globals_iEnginePointer->m__timeCode=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<709>");
		if((c_iSystemGui::m__playfield)!=0){
			DBG_BLOCK();
			c_iSystemGui::m_GuiPage(0);
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<711>");
		if(bb_globals_iEnginePointer->m__cold==true){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<712>");
			bb_globals_iEnginePointer->p_OnColdStart();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<713>");
			bb_globals_iEnginePointer->m__cold=false;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<716>");
		if(bb_globals_iEnginePointer->m__started==false){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<717>");
			bb_globals_iEnginePointer->p_OnStart();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<718>");
			bb_globals_iEnginePointer->m__started=true;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<721>");
		if(bb_globals_iEnginePointer->m__paused){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<722>");
			bb_globals_iEnginePointer->p_UpdateSystemGui();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<723>");
			bb_globals_iEnginePointer->p_OnPaused();
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<725>");
			bb_globals_iEnginePointer->p_UpdateLogic();
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<730>");
		if(bb_globals_iEnginePointer->m__cold==true){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<731>");
			bb_globals_iEnginePointer->p_OnColdStart();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<732>");
			bb_globals_iEnginePointer->m__cold=false;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<735>");
		if(bb_globals_iEnginePointer->m__started==false){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<736>");
			bb_globals_iEnginePointer->p_OnStart();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<737>");
			bb_globals_iEnginePointer->m__started=true;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<740>");
		bb_globals_iEnginePointer->p_UpdateFixed();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<744>");
	c_iTask::m_Update(m__taskList);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<746>");
	c_iTask::m_Update(bb_globals_iTaskList);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<748>");
	c_iEngineObject::m_DestroyList2();
}
void c_iEngine::mark(){
	c_iScene::mark();
	gc_mark_q(m__playfieldList);
	gc_mark_q(m__currentPlayfield);
	gc_mark_q(m__taskList);
	gc_mark_q(m__playfieldPointer);
	gc_mark_q(m__collisionReadList);
	gc_mark_q(m__layerPointer);
	gc_mark_q(m__collisionWriteList);
}
String c_iEngine::debug(){
	String t="(iEngine)\n";
	t=c_iScene::debug()+t;
	t+=dbg_decl("_alphaFade",&m__alphaFade);
	t+=dbg_decl("_autoCls",&m__autoCls);
	t+=dbg_decl("_borders",&m__borders);
	t+=dbg_decl("_clsBlue",&m__clsBlue);
	t+=dbg_decl("_clsGreen",&m__clsGreen);
	t+=dbg_decl("_clsRed",&m__clsRed);
	t+=dbg_decl("_collisionReadList",&m__collisionReadList);
	t+=dbg_decl("_collisionWriteList",&m__collisionWriteList);
	t+=dbg_decl("_colorFade",&m__colorFade);
	t+=dbg_decl("_countDown",&m__countDown);
	t+=dbg_decl("_currentPlayfield",&m__currentPlayfield);
	t+=dbg_decl("_layerPointer",&m__layerPointer);
	t+=dbg_decl("_playfieldList",&m__playfieldList);
	t+=dbg_decl("_playfieldPointer",&m__playfieldPointer);
	t+=dbg_decl("_renderToPlayfield",&m__renderToPlayfield);
	t+=dbg_decl("_scoreCount",&m__scoreCount);
	t+=dbg_decl("_showSystemGui",&m__showSystemGui);
	t+=dbg_decl("_taskList",&m__taskList);
	t+=dbg_decl("_timeCode",&m__timeCode);
	return t;
}
c_MenuScene::c_MenuScene(){
}
c_MenuScene* c_MenuScene::m_new(){
	DBG_ENTER("MenuScene.new")
	c_MenuScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<51>");
	c_iEngine::m_new();
	return this;
}
int c_MenuScene::p_OnCreate(){
	DBG_ENTER("MenuScene.OnCreate")
	c_MenuScene *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_MenuScene::p_OnRender(){
	DBG_ENTER("MenuScene.OnRender")
	c_MenuScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<61>");
	bb_graphics_DrawText(String(L"Press Enter to generate Cellularly",34),FLOAT(200.0),FLOAT(200.0),FLOAT(0.0),FLOAT(0.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<62>");
	bb_graphics_DrawText(String(L"Press Space to generate with Noise",34),FLOAT(200.0),FLOAT(300.0),FLOAT(0.0),FLOAT(0.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<63>");
	bb_graphics_DrawText(String(L"Press Z to demo battles",23),FLOAT(200.0),FLOAT(400.0),FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int c_MenuScene::p_OnStart(){
	DBG_ENTER("MenuScene.OnStart")
	c_MenuScene *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_MenuScene::p_OnStop(){
	DBG_ENTER("MenuScene.OnStop")
	c_MenuScene *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_MenuScene::p_OnUpdate(){
	DBG_ENTER("MenuScene.OnUpdate")
	c_MenuScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<82>");
	if((bb_input_KeyHit(13))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<83>");
		bbPrint(String(L"Switch",6));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<84>");
		bb_app2_iStart2(bb_main_gameplay);
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<87>");
	if((bb_input_KeyHit(32))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<88>");
		bbPrint(String(L"Switch to Noise",15));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<89>");
		bb_app2_iStart2(bb_main_noiseTest);
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<92>");
	if((bb_input_KeyHit(90))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<93>");
		bbPrint(String(L"Switch to Battle",16));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<94>");
		bb_app2_iStart2((new c_BattleScene)->m_new());
	}
	return 0;
}
void c_MenuScene::mark(){
	c_iEngine::mark();
}
String c_MenuScene::debug(){
	String t="(MenuScene)\n";
	t=c_iEngine::debug()+t;
	return t;
}
c_MenuScene* bb_main_menu;
c_GameplayScene::c_GameplayScene(){
	m_playfield=0;
	m_layer=0;
	m_sprite1=0;
	m_p1=0;
	m_music=0;
	m_room=0;
	m_bBox=0;
}
c_GameplayScene* c_GameplayScene::m_new(){
	DBG_ENTER("GameplayScene.new")
	c_GameplayScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<18>");
	c_iEngine::m_new();
	return this;
}
int c_GameplayScene::p_OnCreate(){
	DBG_ENTER("GameplayScene.OnCreate")
	c_GameplayScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<33>");
	bbPrint(String(L"Creating Gameplay",17));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<34>");
	gc_assign(this->m_playfield,(new c_iPlayfield)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<35>");
	this->m_playfield->p_AttachLast();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<36>");
	this->m_playfield->p_AutoCls(0,0,0);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<37>");
	this->m_playfield->p_Width2(FLOAT(600.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<38>");
	this->m_playfield->p_Height2(FLOAT(440.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<39>");
	this->m_playfield->p_Position(FLOAT(16.0),FLOAT(16.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<40>");
	this->m_playfield->p_ZoomPointX2(FLOAT(200.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<41>");
	this->m_playfield->p_ZoomPointY2(FLOAT(128.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<55>");
	gc_assign(this->m_layer,(new c_iLayer)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<59>");
	c_Image* t_img=bb_gfx_iLoadSprite2(String(L"char_walk_down.png",18),69,102,4);
	DBG_LOCAL(t_img,"img")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<60>");
	this->m_layer->p_AttachLast3(this->m_playfield);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<64>");
	gc_assign(this->m_sprite1,(new c_iLayerSprite)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<65>");
	this->m_sprite1->p_AttachLast4(this->m_layer);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<66>");
	this->m_sprite1->p_ImagePointer2(t_img);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<67>");
	this->m_sprite1->p_Position(FLOAT(300.0),FLOAT(275.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<75>");
	gc_assign(this->m_p1,(new c_Player)->m_new(t_img,100,100));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<77>");
	gc_assign(m_music,bb_audio_LoadSound(String(L"tetris.mp3",10)));
	return 0;
}
int c_GameplayScene::p_OnRender(){
	DBG_ENTER("GameplayScene.OnRender")
	c_GameplayScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<85>");
	m_room->p_Draw(int(this->m_playfield->p_CameraX()),int(this->m_playfield->p_CameraY()));
	return 0;
}
int c_GameplayScene::p_OnStart(){
	DBG_ENTER("GameplayScene.OnStart")
	c_GameplayScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<93>");
	gc_assign(m_room,(new c_Level)->m_new(0,0,160,120,String(L"Cellular",8)));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<133>");
	this->m_sprite1->p_Position(Float(m_room->m_entranceX*40),Float(m_room->m_entranceY*40-80));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<135>");
	gc_assign(this->m_bBox,(new c_BoundingRect)->m_new(m_room->m_entranceX*40-5,m_room->m_entranceY*40-60,30,30));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<136>");
	this->m_playfield->p_CameraX2(Float(m_room->m_entranceX*40-300));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<137>");
	this->m_playfield->p_CameraY2(Float(m_room->m_entranceY*40-200));
	return 0;
}
int c_GameplayScene::p_OnStop(){
	DBG_ENTER("GameplayScene.OnStop")
	c_GameplayScene *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_GameplayScene::p_checkCameraBounds(){
	DBG_ENTER("GameplayScene.checkCameraBounds")
	c_GameplayScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<226>");
	if(this->m_playfield->p_CameraX()<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<227>");
		this->m_playfield->p_CameraX2(FLOAT(0.0));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<229>");
	if(this->m_playfield->p_CameraY()<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<230>");
		this->m_playfield->p_CameraY2(FLOAT(0.0));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<237>");
	if(this->m_playfield->p_CameraX()+this->m_playfield->p_Width()>FLOAT(6400.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<238>");
		this->m_playfield->p_CameraX2(FLOAT(6400.0)-this->m_playfield->p_Width());
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<240>");
	if(this->m_playfield->p_CameraY()+this->m_playfield->p_Height()>FLOAT(4800.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<241>");
		this->m_playfield->p_CameraY2(FLOAT(4800.0)-this->m_playfield->p_Height());
	}
	return 0;
}
int c_GameplayScene::p_OnUpdate(){
	DBG_ENTER("GameplayScene.OnUpdate")
	c_GameplayScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<151>");
	if((bb_input_KeyDown(17))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<153>");
		if((bb_input_KeyDown(65))!=0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<154>");
			this->m_playfield->p_AlphaFade2(this->m_playfield->p_AlphaFade()-FLOAT(0.01));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<155>");
			if(this->m_playfield->p_AlphaFade()<FLOAT(0.0)){
				DBG_BLOCK();
				this->m_playfield->p_AlphaFade2(this->m_playfield->p_AlphaFade()+FLOAT(1.0));
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<158>");
		if((bb_input_KeyDown(90))!=0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<159>");
			this->m_playfield->p_ZoomX2(this->m_playfield->p_ZoomX()-FLOAT(0.01));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<160>");
			this->m_playfield->p_ZoomY2(this->m_playfield->p_ZoomY()-FLOAT(0.01));
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<165>");
		if((bb_input_KeyDown(65))!=0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<166>");
			this->m_playfield->p_AlphaFade2(this->m_playfield->p_AlphaFade()+FLOAT(0.01));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<167>");
			if(this->m_playfield->p_AlphaFade()>FLOAT(1.0)){
				DBG_BLOCK();
				this->m_playfield->p_AlphaFade2(this->m_playfield->p_AlphaFade()-FLOAT(1.0));
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<170>");
		if((bb_input_KeyDown(90))!=0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<171>");
			this->m_playfield->p_ZoomX2(this->m_playfield->p_ZoomX()+FLOAT(0.01));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<172>");
			this->m_playfield->p_ZoomY2(this->m_playfield->p_ZoomY()+FLOAT(0.01));
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<176>");
	if((bb_input_KeyDown(37))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<177>");
		if(!(m_room->m_layout.At((m_bBox->m_x-5)/40).At(m_bBox->m_y/40)==0 || m_room->m_layout.At((m_bBox->m_x-5)/40).At((m_bBox->m_y+m_bBox->m_height)/40)==0)){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<178>");
			if(m_sprite1->p_PositionX()-this->m_playfield->p_CameraX()<FLOAT(200.0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<179>");
				this->m_playfield->p_CameraX2(this->m_playfield->p_CameraX()-FLOAT(5.0));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<182>");
			m_sprite1->p_PositionX2(m_sprite1->p_PositionX()-FLOAT(5.0));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<183>");
			this->m_bBox->m_x-=5;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<187>");
	if((bb_input_KeyDown(39))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<188>");
		if(!(m_room->m_layout.At((m_bBox->m_x+m_bBox->m_width+5)/40).At(m_bBox->m_y/40)==0 || m_room->m_layout.At((m_bBox->m_x+m_bBox->m_width+5)/40).At((m_bBox->m_y+m_bBox->m_height)/40)==0)){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<189>");
			if(m_sprite1->p_PositionX()-this->m_playfield->p_CameraX()>FLOAT(350.0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<190>");
				this->m_playfield->p_CameraX2(this->m_playfield->p_CameraX()+FLOAT(5.0));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<192>");
			m_sprite1->p_PositionX2(m_sprite1->p_PositionX()+FLOAT(5.0));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<193>");
			this->m_bBox->m_x+=5;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<197>");
	if((bb_input_KeyDown(38))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<198>");
		if(!(m_room->m_layout.At(m_bBox->m_x/40).At((m_bBox->m_y-5)/40)==0 || m_room->m_layout.At((m_bBox->m_x+m_bBox->m_width)/40).At((m_bBox->m_y-5)/40)==0)){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<199>");
			if(m_sprite1->p_PositionY()-this->m_playfield->p_CameraY()<FLOAT(200.0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<200>");
				this->m_playfield->p_CameraY2(this->m_playfield->p_CameraY()-FLOAT(5.0));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<202>");
			m_sprite1->p_PositionY2(m_sprite1->p_PositionY()-FLOAT(5.0));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<203>");
			this->m_bBox->m_y-=5;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<207>");
	if((bb_input_KeyDown(40))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<208>");
		if(!(m_room->m_layout.At(m_bBox->m_x/40).At((m_bBox->m_y+m_bBox->m_height+5)/40)==0 || m_room->m_layout.At((m_bBox->m_x+m_bBox->m_width)/40).At((m_bBox->m_y+m_bBox->m_height+5)/40)==0)){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<209>");
			if(m_sprite1->p_PositionY()-this->m_playfield->p_CameraY()>FLOAT(250.0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<210>");
				this->m_playfield->p_CameraY2(this->m_playfield->p_CameraY()+FLOAT(5.0));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<213>");
			m_sprite1->p_PositionY2(m_sprite1->p_PositionY()+FLOAT(5.0));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<214>");
			this->m_bBox->m_y+=5;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<217>");
	p_checkCameraBounds();
	return 0;
}
void c_GameplayScene::mark(){
	c_iEngine::mark();
	gc_mark_q(m_playfield);
	gc_mark_q(m_layer);
	gc_mark_q(m_sprite1);
	gc_mark_q(m_p1);
	gc_mark_q(m_music);
	gc_mark_q(m_room);
	gc_mark_q(m_bBox);
}
String c_GameplayScene::debug(){
	String t="(GameplayScene)\n";
	t=c_iEngine::debug()+t;
	t+=dbg_decl("p1",&m_p1);
	t+=dbg_decl("music",&m_music);
	t+=dbg_decl("room",&m_room);
	t+=dbg_decl("playfield",&m_playfield);
	t+=dbg_decl("layer",&m_layer);
	t+=dbg_decl("sprite1",&m_sprite1);
	t+=dbg_decl("bBox",&m_bBox);
	return t;
}
c_GameplayScene* bb_main_gameplay;
c_NoiseTestScene::c_NoiseTestScene(){
	m_playfieldN=0;
	m_mapWidth=0;
	m_mapHeight=0;
	m_layer=0;
	m_sprite1=0;
	m_deepWaterTiles=Array<c_Point* >();
	m_shallowWaterTiles=Array<c_Point* >();
	m_beachTiles=Array<c_Point* >();
	m_lightGrassTiles=Array<c_Point* >();
	m_heavyGrassTiles=Array<c_Point* >();
	m_swampTiles=Array<c_Point* >();
	m_forestTiles=Array<c_Point* >();
	m_desertTiles=Array<c_Point* >();
	m_mountainTiles=Array<c_Point* >();
	m_darkSnowMountainTiles=Array<c_Point* >();
	m_lightSnowMountainTiles=Array<c_Point* >();
	m_riverTiles=Array<c_Point* >();
	m_caveEntranceTiles=Array<c_Point* >();
	m_biomes=Array<Array<int > >();
	m_enemyPlacement=Array<Array<int > >();
	m_noiseMap=Array<Array<Float > >();
	m_moisture=Array<Array<Float > >();
	m_tileCounter=0;
	m_startX=0;
	m_startY=0;
	m_bBox=0;
	m_caves=Array<c_Level* >(1);
	m_caveX=0;
	m_caveY=0;
}
c_NoiseTestScene* c_NoiseTestScene::m_new(){
	DBG_ENTER("NoiseTestScene.new")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<15>");
	c_iEngine::m_new();
	return this;
}
int c_NoiseTestScene::p_OnCreate(){
	DBG_ENTER("NoiseTestScene.OnCreate")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<104>");
	gc_assign(this->m_playfieldN,(new c_iPlayfield)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<105>");
	this->m_playfieldN->p_AttachLast();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<106>");
	this->m_playfieldN->p_AutoCls(0,0,0);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<107>");
	this->m_playfieldN->p_Width2(FLOAT(600.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<108>");
	this->m_playfieldN->p_Height2(FLOAT(460.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<109>");
	this->m_playfieldN->p_Position(FLOAT(25.0),FLOAT(15.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<110>");
	this->m_playfieldN->p_ZoomPointX2(FLOAT(200.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<111>");
	this->m_playfieldN->p_ZoomPointY2(FLOAT(128.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<113>");
	gc_assign(bb_noisetestscene_textures,bb_gfx_iLoadSprite2(String(L"textures40.png",14),40,40,32));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<114>");
	gc_assign(bb_noisetestscene_enemies,bb_gfx_iLoadSprite2(String(L"enemies40.png",13),40,40,9));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<121>");
	m_mapWidth=600;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<122>");
	m_mapHeight=460;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<124>");
	gc_assign(this->m_layer,(new c_iLayer)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<125>");
	c_Image* t_img=bb_gfx_iLoadSprite2(String(L"char_walk_down.png",18),69,102,4);
	DBG_LOCAL(t_img,"img")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<126>");
	this->m_layer->p_AttachLast3(this->m_playfieldN);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<128>");
	gc_assign(this->m_sprite1,(new c_iLayerSprite)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<129>");
	this->m_sprite1->p_AttachLast4(this->m_layer);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<130>");
	this->m_sprite1->p_ImagePointer2(t_img);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<131>");
	this->m_sprite1->p_Position(FLOAT(300.0),FLOAT(275.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<134>");
	gc_assign(this->m_deepWaterTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<135>");
	gc_assign(this->m_shallowWaterTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<136>");
	gc_assign(this->m_beachTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<137>");
	gc_assign(this->m_lightGrassTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<138>");
	gc_assign(this->m_heavyGrassTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<139>");
	gc_assign(this->m_swampTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<140>");
	gc_assign(this->m_forestTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<141>");
	gc_assign(this->m_desertTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<142>");
	gc_assign(this->m_mountainTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<143>");
	gc_assign(this->m_darkSnowMountainTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<144>");
	gc_assign(this->m_lightSnowMountainTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<145>");
	gc_assign(this->m_riverTiles,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<146>");
	gc_assign(this->m_caveEntranceTiles,Array<c_Point* >(1000));
	return 0;
}
int c_NoiseTestScene::p_drawNoiseMap(int t_w,int t_h){
	DBG_ENTER("NoiseTestScene.drawNoiseMap")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_w,"w")
	DBG_LOCAL(t_h,"h")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<668>");
	int t_xTile=int(this->m_playfieldN->p_CameraX()/FLOAT(40.0)-FLOAT(1.0));
	DBG_LOCAL(t_xTile,"xTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<669>");
	int t_yTile=int(this->m_playfieldN->p_CameraY()/FLOAT(40.0)-FLOAT(1.0));
	DBG_LOCAL(t_yTile,"yTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<670>");
	int t_xOffset=int((Float)fmod(this->m_playfieldN->p_CameraX(),FLOAT(40.0)));
	DBG_LOCAL(t_xOffset,"xOffset")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<671>");
	int t_yOffset=int((Float)fmod(this->m_playfieldN->p_CameraY(),FLOAT(40.0)));
	DBG_LOCAL(t_yOffset,"yOffset")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<673>");
	for(int t_i=t_xTile;t_i<t_xTile+18;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<674>");
		for(int t_j=t_yTile;t_j<t_yTile+14;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<675>");
			if(t_i>-1 && t_i<m_biomes.Length() && t_j>-1 && t_j<m_biomes.At(t_i).Length()){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<676>");
				bb_graphics_DrawImage(bb_noisetestscene_textures,Float(t_i*40),Float(t_j*40),m_biomes.At(t_i).At(t_j));
			}
		}
	}
	return 0;
}
int c_NoiseTestScene::p_drawEnemies(int t_w,int t_h){
	DBG_ENTER("NoiseTestScene.drawEnemies")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_w,"w")
	DBG_LOCAL(t_h,"h")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<692>");
	int t_xTile=int(this->m_playfieldN->p_CameraX()/FLOAT(40.0)-FLOAT(1.0));
	DBG_LOCAL(t_xTile,"xTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<693>");
	int t_yTile=int(this->m_playfieldN->p_CameraY()/FLOAT(40.0)-FLOAT(1.0));
	DBG_LOCAL(t_yTile,"yTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<694>");
	int t_xOffset=int((Float)fmod(this->m_playfieldN->p_CameraX(),FLOAT(40.0)));
	DBG_LOCAL(t_xOffset,"xOffset")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<695>");
	int t_yOffset=int((Float)fmod(this->m_playfieldN->p_CameraY(),FLOAT(40.0)));
	DBG_LOCAL(t_yOffset,"yOffset")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<697>");
	for(int t_i=t_xTile;t_i<t_xTile+18;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<698>");
		for(int t_j=t_yTile;t_j<t_yTile+14;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<699>");
			if(!(m_enemyPlacement.At(t_i).At(t_j)==-1)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<700>");
				bb_graphics_DrawImage(bb_noisetestscene_enemies,Float(t_i*40),Float(t_j*40),m_enemyPlacement.At(t_i).At(t_j));
			}
		}
	}
	return 0;
}
int c_NoiseTestScene::p_OnRender(){
	DBG_ENTER("NoiseTestScene.OnRender")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<152>");
	p_drawNoiseMap(m_mapWidth,m_mapHeight);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<153>");
	p_drawEnemies(m_mapWidth,m_mapHeight);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<155>");
	bb_graphics_DrawText(String(L"CameraX: ",9)+String(this->m_playfieldN->p_CameraX()),this->m_playfieldN->p_CameraX()+FLOAT(10.0),this->m_playfieldN->p_CameraY()+FLOAT(10.0),FLOAT(0.0),FLOAT(0.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<156>");
	bb_graphics_DrawText(String(L"CameraY: ",9)+String(this->m_playfieldN->p_CameraY()),this->m_playfieldN->p_CameraX()+FLOAT(10.0),this->m_playfieldN->p_CameraY()+FLOAT(20.0),FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int c_NoiseTestScene::p_determineBiomes(Array<Array<Float > > t_elevation,Array<Array<Float > > t_moist,int t_width,int t_height){
	DBG_ENTER("NoiseTestScene.determineBiomes")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_elevation,"elevation")
	DBG_LOCAL(t_moist,"moist")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<710>");
	int t_localBiome=0;
	DBG_LOCAL(t_localBiome,"localBiome")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<712>");
	for(int t_i=0;t_i<t_width;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<713>");
		for(int t_j=0;t_j<t_height;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<714>");
			if(m_noiseMap.At(t_i).At(t_j)<FLOAT(-0.35)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<715>");
				t_localBiome=0;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<716>");
				if(m_noiseMap.At(t_i).At(t_j)<FLOAT(-0.3)){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<717>");
					t_localBiome=1;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<718>");
					if(m_noiseMap.At(t_i).At(t_j)<FLOAT(-0.2)){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<719>");
						t_localBiome=2;
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<723>");
						if(m_noiseMap.At(t_i).At(t_j)<FLOAT(0.2)){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<724>");
							if(m_moisture.At(t_i).At(t_j)<FLOAT(-0.6)){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<725>");
								t_localBiome=7;
							}else{
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<726>");
								if(m_moisture.At(t_i).At(t_j)<FLOAT(-0.1)){
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<727>");
									t_localBiome=3;
								}else{
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<728>");
									if(m_moisture.At(t_i).At(t_j)<FLOAT(0.2)){
										DBG_BLOCK();
										DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<729>");
										t_localBiome=4;
									}else{
										DBG_BLOCK();
										DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<730>");
										if(m_moisture.At(t_i).At(t_j)<FLOAT(0.4)){
											DBG_BLOCK();
											DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<731>");
											t_localBiome=6;
										}else{
											DBG_BLOCK();
											DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<732>");
											if(m_moisture.At(t_i).At(t_j)<FLOAT(1.0)){
												DBG_BLOCK();
												DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<733>");
												t_localBiome=5;
											}
										}
									}
								}
							}
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<739>");
							if(m_moisture.At(t_i).At(t_j)<FLOAT(-0.4)){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<740>");
								t_localBiome=8;
							}else{
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<741>");
								if(m_moisture.At(t_i).At(t_j)<FLOAT(0.0)){
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<742>");
									t_localBiome=21;
								}else{
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<744>");
									t_localBiome=22;
								}
							}
						}
					}
				}
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<747>");
			if(t_i==0 || t_i==t_width-1 || t_j==0 || t_j==t_height-1){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<748>");
				t_localBiome=11;
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<750>");
			m_biomes.At(t_i).At(t_j)=t_localBiome;
		}
	}
	return 0;
}
int c_NoiseTestScene::p_processBiomes(){
	DBG_ENTER("NoiseTestScene.processBiomes")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<760>");
	int t_localBiome=0;
	DBG_LOCAL(t_localBiome,"localBiome")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<762>");
	int t_deepWaterIndex=0;
	DBG_LOCAL(t_deepWaterIndex,"deepWaterIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<763>");
	int t_shallowWaterIndex=0;
	DBG_LOCAL(t_shallowWaterIndex,"shallowWaterIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<764>");
	int t_beachIndex=0;
	DBG_LOCAL(t_beachIndex,"beachIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<765>");
	int t_lightGrassIndex=0;
	DBG_LOCAL(t_lightGrassIndex,"lightGrassIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<766>");
	int t_heavyGrassIndex=0;
	DBG_LOCAL(t_heavyGrassIndex,"heavyGrassIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<767>");
	int t_swampIndex=0;
	DBG_LOCAL(t_swampIndex,"swampIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<768>");
	int t_forestIndex=0;
	DBG_LOCAL(t_forestIndex,"forestIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<769>");
	int t_desertIndex=0;
	DBG_LOCAL(t_desertIndex,"desertIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<770>");
	int t_mountainIndex=0;
	DBG_LOCAL(t_mountainIndex,"mountainIndex")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<772>");
	for(int t_i=0;t_i<this->m_mapWidth;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<773>");
		for(int t_j=0;t_j<this->m_mapHeight;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<775>");
			t_localBiome=this->m_biomes.At(t_i).At(t_j);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<776>");
			if(t_localBiome==0){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<777>");
				gc_assign(m_deepWaterTiles.At(t_deepWaterIndex),(new c_Point)->m_new(t_i,t_j));
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<778>");
				t_deepWaterIndex+=1;
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<779>");
				if(t_deepWaterIndex==m_deepWaterTiles.Length()){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<780>");
					gc_assign(m_deepWaterTiles,m_deepWaterTiles.Resize(m_deepWaterTiles.Length()+1000));
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<782>");
				if(t_localBiome==1){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<783>");
					gc_assign(m_shallowWaterTiles.At(t_shallowWaterIndex),(new c_Point)->m_new(t_i,t_j));
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<784>");
					t_shallowWaterIndex+=1;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<785>");
					if(t_shallowWaterIndex==m_shallowWaterTiles.Length()){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<786>");
						gc_assign(m_shallowWaterTiles,m_shallowWaterTiles.Resize(m_shallowWaterTiles.Length()+1000));
					}
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<788>");
					if(t_localBiome==2){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<789>");
						gc_assign(m_beachTiles.At(t_beachIndex),(new c_Point)->m_new(t_i,t_j));
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<790>");
						t_beachIndex+=1;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<791>");
						if(t_beachIndex==m_beachTiles.Length()){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<792>");
							gc_assign(m_beachTiles,m_beachTiles.Resize(m_beachTiles.Length()+1000));
						}
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<794>");
						if(t_localBiome==3){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<795>");
							gc_assign(m_lightGrassTiles.At(t_lightGrassIndex),(new c_Point)->m_new(t_i,t_j));
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<796>");
							t_lightGrassIndex+=1;
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<797>");
							if(t_lightGrassIndex==m_lightGrassTiles.Length()){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<798>");
								gc_assign(m_lightGrassTiles,m_lightGrassTiles.Resize(m_lightGrassTiles.Length()+1000));
							}
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<800>");
							if(t_localBiome==4){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<801>");
								gc_assign(m_heavyGrassTiles.At(t_heavyGrassIndex),(new c_Point)->m_new(t_i,t_j));
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<802>");
								t_heavyGrassIndex+=1;
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<803>");
								if(t_heavyGrassIndex==m_heavyGrassTiles.Length()){
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<804>");
									gc_assign(m_heavyGrassTiles,m_heavyGrassTiles.Resize(m_heavyGrassTiles.Length()+1000));
								}
							}else{
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<806>");
								if(t_localBiome==5){
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<807>");
									gc_assign(m_swampTiles.At(t_swampIndex),(new c_Point)->m_new(t_i,t_j));
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<808>");
									t_swampIndex+=1;
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<809>");
									if(t_swampIndex==m_swampTiles.Length()){
										DBG_BLOCK();
										DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<810>");
										gc_assign(m_swampTiles,m_swampTiles.Resize(m_swampTiles.Length()+1000));
									}
								}else{
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<812>");
									if(t_localBiome==6){
										DBG_BLOCK();
										DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<813>");
										gc_assign(m_forestTiles.At(t_forestIndex),(new c_Point)->m_new(t_i,t_j));
										DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<814>");
										t_forestIndex+=1;
										DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<815>");
										if(t_forestIndex==m_forestTiles.Length()){
											DBG_BLOCK();
											DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<816>");
											gc_assign(m_forestTiles,m_forestTiles.Resize(m_forestTiles.Length()+1000));
										}
									}else{
										DBG_BLOCK();
										DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<818>");
										if(t_localBiome==7){
											DBG_BLOCK();
											DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<819>");
											gc_assign(m_desertTiles.At(t_desertIndex),(new c_Point)->m_new(t_i,t_j));
											DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<820>");
											t_desertIndex+=1;
											DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<821>");
											if(t_desertIndex==m_desertTiles.Length()){
												DBG_BLOCK();
												DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<822>");
												gc_assign(m_desertTiles,m_desertTiles.Resize(m_desertTiles.Length()+1000));
											}
										}else{
											DBG_BLOCK();
											DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<824>");
											if(t_localBiome==8){
												DBG_BLOCK();
												DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<825>");
												gc_assign(m_mountainTiles.At(t_mountainIndex),(new c_Point)->m_new(t_i,t_j));
												DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<826>");
												t_mountainIndex+=1;
												DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<827>");
												if(t_mountainIndex==m_mountainTiles.Length()){
													DBG_BLOCK();
													DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<828>");
													gc_assign(m_mountainTiles,m_mountainTiles.Resize(m_mountainTiles.Length()+1000));
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<833>");
	gc_assign(m_deepWaterTiles,m_deepWaterTiles.Resize(t_deepWaterIndex));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<834>");
	gc_assign(m_shallowWaterTiles,m_shallowWaterTiles.Resize(t_shallowWaterIndex));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<835>");
	gc_assign(m_beachTiles,m_beachTiles.Resize(t_beachIndex));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<836>");
	gc_assign(m_lightGrassTiles,m_lightGrassTiles.Resize(t_lightGrassIndex));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<837>");
	gc_assign(m_heavyGrassTiles,m_heavyGrassTiles.Resize(t_heavyGrassIndex));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<838>");
	gc_assign(m_swampTiles,m_swampTiles.Resize(t_swampIndex));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<839>");
	gc_assign(m_forestTiles,m_forestTiles.Resize(t_forestIndex));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<840>");
	gc_assign(m_desertTiles,m_desertTiles.Resize(t_desertIndex));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<841>");
	gc_assign(m_mountainTiles,m_mountainTiles.Resize(t_mountainIndex));
	return 0;
}
int c_NoiseTestScene::p_detailBiomes(){
	DBG_ENTER("NoiseTestScene.detailBiomes")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<200>");
	int t_cacti=m_desertTiles.Length()/20;
	DBG_LOCAL(t_cacti,"cacti")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<201>");
	int t_counter=0;
	DBG_LOCAL(t_counter,"counter")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<202>");
	int t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_desertTiles.Length())));
	DBG_LOCAL(t_randTile,"randTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<204>");
	int t_x=0;
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<205>");
	int t_y=0;
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<206>");
	while(t_counter<t_cacti){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<207>");
		t_x=m_desertTiles.At(t_randTile)->p_getX();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<208>");
		t_y=m_desertTiles.At(t_randTile)->p_getY();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<209>");
		if(m_biomes.At(t_x).At(t_y)==7){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<210>");
			m_biomes.At(t_x).At(t_y)=19;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<211>");
			t_counter+=1;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<213>");
		t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_desertTiles.Length())));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<216>");
	int t_beachItems=m_beachTiles.Length()/30;
	DBG_LOCAL(t_beachItems,"beachItems")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<217>");
	t_counter=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<218>");
	t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_beachTiles.Length())));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<220>");
	t_x=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<221>");
	t_y=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<222>");
	while(t_counter<t_beachItems){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<223>");
		t_x=m_beachTiles.At(t_randTile)->p_getX();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<224>");
		t_y=m_beachTiles.At(t_randTile)->p_getY();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<225>");
		if(m_biomes.At(t_x).At(t_y)==2){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<226>");
			m_biomes.At(t_x).At(t_y)=20;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<227>");
			t_counter+=1;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<229>");
		t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_beachTiles.Length())));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<232>");
	int t_rocks=m_lightGrassTiles.Length()/40;
	DBG_LOCAL(t_rocks,"rocks")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<233>");
	t_counter=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<234>");
	t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_lightGrassTiles.Length())));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<236>");
	t_x=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<237>");
	t_y=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<239>");
	while(t_counter<t_rocks){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<240>");
		t_x=m_lightGrassTiles.At(t_randTile)->p_getX();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<241>");
		t_y=m_lightGrassTiles.At(t_randTile)->p_getY();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<242>");
		if(m_biomes.At(t_x).At(t_y)==3){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<243>");
			m_biomes.At(t_x).At(t_y)=31;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<244>");
			t_counter+=1;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<246>");
		t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_lightGrassTiles.Length())));
	}
	return 0;
}
int c_NoiseTestScene::p_makeLake(int t_centerX,int t_centerY){
	DBG_ENTER("NoiseTestScene.makeLake")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_centerX,"centerX")
	DBG_LOCAL(t_centerY,"centerY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<420>");
	int t_lakeWidth=int(bb_random_Rnd2(FLOAT(1.0),FLOAT(5.0)));
	DBG_LOCAL(t_lakeWidth,"lakeWidth")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<421>");
	int t_lakeHeight=int(bb_random_Rnd2(FLOAT(1.0),FLOAT(5.0)));
	DBG_LOCAL(t_lakeHeight,"lakeHeight")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<423>");
	int t_tempX=t_centerX-t_lakeWidth;
	DBG_LOCAL(t_tempX,"tempX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<424>");
	int t_tempY=t_centerY-t_lakeHeight;
	DBG_LOCAL(t_tempY,"tempY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<426>");
	int t_xTarget=t_tempX+(t_lakeWidth*2+1);
	DBG_LOCAL(t_xTarget,"xTarget")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<427>");
	int t_yTarget=t_tempY+(t_lakeHeight*2+1);
	DBG_LOCAL(t_yTarget,"yTarget")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<429>");
	m_biomes.At(t_centerX).At(t_centerY)=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<431>");
	for(int t_i=t_tempX;t_i<t_xTarget;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<432>");
		for(int t_j=t_tempY;t_j<t_yTarget;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<433>");
			if(t_i>0 && t_i<m_mapWidth-1 && t_j>0 && t_j<m_mapHeight-1 && !(m_biomes.At(t_i).At(t_j)==0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<434>");
				if(t_i==t_centerX-t_lakeWidth){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<435>");
					if(t_j==t_centerY-t_lakeHeight){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<436>");
						m_biomes.At(t_i).At(t_j)=23;
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<437>");
						if(t_j==t_yTarget-1){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<438>");
							m_biomes.At(t_i).At(t_j)=28;
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<440>");
							m_biomes.At(t_i).At(t_j)=26;
						}
					}
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<442>");
					if(t_i==t_xTarget-1){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<443>");
						if(t_j==t_centerY-t_lakeHeight){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<444>");
							m_biomes.At(t_i).At(t_j)=25;
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<445>");
							if(t_j==t_yTarget-1){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<446>");
								m_biomes.At(t_i).At(t_j)=30;
							}else{
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<448>");
								m_biomes.At(t_i).At(t_j)=27;
							}
						}
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<450>");
						if(t_j==t_centerY-t_lakeHeight){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<451>");
							m_biomes.At(t_i).At(t_j)=24;
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<452>");
							if(t_j==t_yTarget-1){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<453>");
								m_biomes.At(t_i).At(t_j)=29;
							}else{
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<455>");
								m_biomes.At(t_i).At(t_j)=0;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
int c_NoiseTestScene::p_makeRiver(int t_startX,int t_startY){
	DBG_ENTER("NoiseTestScene.makeRiver")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_startX,"startX")
	DBG_LOCAL(t_startY,"startY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<273>");
	int t_currentX=t_startX;
	DBG_LOCAL(t_currentX,"currentX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<274>");
	int t_currentY=t_startY;
	DBG_LOCAL(t_currentY,"currentY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<275>");
	int t_localMinX=t_startX;
	DBG_LOCAL(t_localMinX,"localMinX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<276>");
	int t_localMinY=t_startY;
	DBG_LOCAL(t_localMinY,"localMinY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<277>");
	Float t_lowestElevation=m_noiseMap.At(t_currentX).At(t_currentY);
	DBG_LOCAL(t_lowestElevation,"lowestElevation")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<278>");
	bool t_riverEnd=false;
	DBG_LOCAL(t_riverEnd,"riverEnd")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<279>");
	int t_riverLength=0;
	DBG_LOCAL(t_riverLength,"riverLength")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<281>");
	int t_up=0;
	DBG_LOCAL(t_up,"up")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<282>");
	int t_right=1;
	DBG_LOCAL(t_right,"right")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<283>");
	int t_down=2;
	DBG_LOCAL(t_down,"down")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<284>");
	int t_left=3;
	DBG_LOCAL(t_left,"left")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<287>");
	int t_previousDirection=-1;
	DBG_LOCAL(t_previousDirection,"previousDirection")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<288>");
	int t_direction=0;
	DBG_LOCAL(t_direction,"direction")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<290>");
	int t_riverType=0;
	DBG_LOCAL(t_riverType,"riverType")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<292>");
	while(t_riverEnd==false && t_riverLength<100){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<294>");
		if(t_currentX>0 && m_noiseMap.At(t_currentX-1).At(t_currentY)<t_lowestElevation){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<295>");
			t_localMinX=t_currentX-1;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<296>");
			t_localMinY=t_currentY;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<297>");
			t_lowestElevation=m_noiseMap.At(t_localMinX).At(t_localMinY);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<298>");
			t_direction=t_left;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<300>");
		if(t_currentX<m_mapWidth-2 && m_noiseMap.At(t_currentX+1).At(t_currentY)<t_lowestElevation){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<301>");
			t_localMinX=t_currentX+1;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<302>");
			t_localMinY=t_currentY;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<303>");
			t_lowestElevation=m_noiseMap.At(t_localMinX).At(t_localMinY);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<304>");
			t_direction=t_right;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<306>");
		if(t_currentY>0 && m_noiseMap.At(t_currentX).At(t_currentY-1)<t_lowestElevation){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<307>");
			t_localMinX=t_currentX;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<308>");
			t_localMinY=t_currentY-1;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<309>");
			t_lowestElevation=m_noiseMap.At(t_localMinX).At(t_localMinY);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<310>");
			t_direction=t_up;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<312>");
		if(t_currentY<m_mapHeight-2 && m_noiseMap.At(t_currentX).At(t_currentY+1)<t_lowestElevation){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<313>");
			t_localMinX=t_currentX;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<314>");
			t_localMinY=t_currentY+1;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<315>");
			t_lowestElevation=m_noiseMap.At(t_localMinX).At(t_localMinY);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<316>");
			t_direction=t_down;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<319>");
		if(t_localMinX==t_currentX && t_localMinY==t_currentY){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<320>");
			int t_decision=int(bb_random_Rnd2(FLOAT(0.0),FLOAT(100.0)));
			DBG_LOCAL(t_decision,"decision")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<321>");
			if(t_decision<15){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<322>");
				p_makeLake(t_currentX,t_currentY);
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<323>");
				t_riverEnd=true;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<325>");
				if(t_currentX-4>0 && m_noiseMap.At(t_currentX-3).At(t_currentY)<t_lowestElevation){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<326>");
					t_localMinX=t_currentX-3;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<327>");
					t_localMinY=t_currentY;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<328>");
					t_lowestElevation=m_noiseMap.At(t_localMinX).At(t_localMinY);
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<329>");
					t_direction=t_left;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<331>");
				if(t_currentX<m_mapWidth-5 && m_noiseMap.At(t_currentX+3).At(t_currentY)<t_lowestElevation){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<332>");
					t_localMinX=t_currentX+3;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<333>");
					t_localMinY=t_currentY;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<334>");
					t_lowestElevation=m_noiseMap.At(t_localMinX).At(t_localMinY);
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<335>");
					t_direction=t_right;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<337>");
				if(t_currentY>0 && m_noiseMap.At(t_currentX).At(t_currentY-3)<t_lowestElevation){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<338>");
					t_localMinX=t_currentX;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<339>");
					t_localMinY=t_currentY-3;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<340>");
					t_lowestElevation=m_noiseMap.At(t_localMinX).At(t_localMinY);
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<341>");
					t_direction=t_up;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<343>");
				if(t_currentY<m_mapHeight-5 && m_noiseMap.At(t_currentX).At(t_currentY+3)<t_lowestElevation){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<344>");
					t_localMinX=t_currentX;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<345>");
					t_localMinY=t_currentY+3;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<346>");
					t_lowestElevation=m_noiseMap.At(t_localMinX).At(t_localMinY);
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<347>");
					t_direction=t_down;
				}
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<351>");
			if(t_direction==t_up){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<352>");
				if(t_previousDirection==t_left){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<353>");
					t_riverType=14;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<354>");
					if(t_previousDirection==t_right){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<355>");
						t_riverType=13;
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<357>");
						t_riverType=12;
					}
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<359>");
				if(t_direction==t_down){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<360>");
					if(t_previousDirection==t_left){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<361>");
						t_riverType=16;
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<362>");
						if(t_previousDirection==t_right){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<363>");
							t_riverType=15;
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<365>");
							t_riverType=12;
						}
					}
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<367>");
					if(t_direction==t_left){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<368>");
						if(t_previousDirection==t_up){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<369>");
							t_riverType=15;
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<370>");
							if(t_previousDirection==t_down){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<371>");
								t_riverType=13;
							}else{
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<373>");
								t_riverType=17;
							}
						}
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<375>");
						if(t_direction==t_right){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<376>");
							if(t_previousDirection==t_up){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<377>");
								t_riverType=16;
							}else{
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<378>");
								if(t_previousDirection==t_down){
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<379>");
									t_riverType=14;
								}else{
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<381>");
									t_riverType=17;
								}
							}
						}
					}
				}
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<384>");
			m_biomes.At(t_currentX).At(t_currentY)=t_riverType;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<385>");
			if(t_localMinX==t_currentX-3){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<386>");
				m_biomes.At(t_currentX-1).At(t_currentY)=17;
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<387>");
				m_biomes.At(t_currentX-2).At(t_currentY)=17;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<388>");
				if(t_localMinX==t_currentX+3){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<389>");
					m_biomes.At(t_currentX+1).At(t_currentY)=17;
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<390>");
					m_biomes.At(t_currentX+2).At(t_currentY)=17;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<391>");
					if(t_localMinY==t_currentY-3){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<392>");
						m_biomes.At(t_currentX).At(t_currentY-1)=12;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<393>");
						m_biomes.At(t_currentX).At(t_currentY-2)=12;
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<394>");
						if(t_localMinY==t_currentY+3){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<395>");
							m_biomes.At(t_currentX).At(t_currentY+1)=12;
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<396>");
							m_biomes.At(t_currentX).At(t_currentY+2)=12;
						}
					}
				}
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<399>");
			if(m_biomes.At(t_localMinX).At(t_localMinY)==0){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<400>");
				t_riverEnd=true;
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<402>");
			gc_assign(m_riverTiles.At(this->m_tileCounter),(new c_Point)->m_new(t_currentX,t_currentY));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<403>");
			this->m_tileCounter+=1;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<404>");
			if(this->m_tileCounter==m_riverTiles.Length()){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<405>");
				gc_assign(m_riverTiles,m_riverTiles.Resize(m_riverTiles.Length()+1000));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<407>");
			t_currentX=t_localMinX;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<408>");
			t_currentY=t_localMinY;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<409>");
			t_riverLength+=1;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<410>");
			t_previousDirection=t_direction;
		}
	}
	return 0;
}
int c_NoiseTestScene::p_makeRivers(int t_total){
	DBG_ENTER("NoiseTestScene.makeRivers")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_total,"total")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<253>");
	this->m_tileCounter=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<254>");
	int t_rivers=0;
	DBG_LOCAL(t_rivers,"rivers")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<255>");
	int t_randX=0;
	DBG_LOCAL(t_randX,"randX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<256>");
	int t_randY=0;
	DBG_LOCAL(t_randY,"randY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<257>");
	int t_randBiome=0;
	DBG_LOCAL(t_randBiome,"randBiome")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<258>");
	while(t_rivers<t_total){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<259>");
		t_randX=int(bb_random_Rnd2(FLOAT(0.0),Float(m_mapWidth)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<260>");
		t_randY=int(bb_random_Rnd2(FLOAT(0.0),Float(m_mapHeight)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<261>");
		t_randBiome=m_biomes.At(t_randX).At(t_randY);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<262>");
		if(t_randBiome==8){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<263>");
			p_makeRiver(t_randX,t_randY);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<264>");
			t_rivers+=1;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<267>");
	gc_assign(m_riverTiles,m_riverTiles.Resize(this->m_tileCounter));
	return 0;
}
int c_NoiseTestScene::p_selectRandomStartPoint(){
	DBG_ENTER("NoiseTestScene.selectRandomStartPoint")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<515>");
	int t_x=0;
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<516>");
	int t_y=0;
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<517>");
	bool t_startPointSet=false;
	DBG_LOCAL(t_startPointSet,"startPointSet")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<519>");
	while(!t_startPointSet){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<520>");
		t_x=int(bb_random_Rnd2(FLOAT(200.0),FLOAT(400.0)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<521>");
		t_y=int(bb_random_Rnd2(FLOAT(130.0),FLOAT(330.0)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<522>");
		if(this->m_biomes.At(t_x).At(t_y)==3 || this->m_biomes.At(t_x).At(t_y)==4){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<523>");
			this->m_playfieldN->p_CameraX2(Float(t_x*40));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<524>");
			this->m_playfieldN->p_CameraY2(Float(t_y*40));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<526>");
			this->m_startX=t_x;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<527>");
			this->m_startY=t_y;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<529>");
			t_startPointSet=true;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<532>");
	this->m_sprite1->p_Position(Float(t_x*40),Float(t_y*40));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<533>");
	this->m_playfieldN->p_CameraX2(m_sprite1->p_PositionX()-FLOAT(300.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<534>");
	this->m_playfieldN->p_CameraY2(m_sprite1->p_PositionY()-FLOAT(235.0));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<535>");
	gc_assign(this->m_bBox,(new c_BoundingRect)->m_new(t_x*40-5,t_y*40+20,30,30));
	return 0;
}
int c_NoiseTestScene::p_makeCaves(int t_total){
	DBG_ENTER("NoiseTestScene.makeCaves")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_total,"total")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<466>");
	int t_lowX=int(this->m_playfieldN->p_CameraX()/FLOAT(40.0)-FLOAT(50.0));
	DBG_LOCAL(t_lowX,"lowX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<467>");
	int t_highX=int(this->m_playfieldN->p_CameraX()/FLOAT(40.0)+FLOAT(100.0));
	DBG_LOCAL(t_highX,"highX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<468>");
	int t_lowY=int(this->m_playfieldN->p_CameraY()/FLOAT(40.0)-FLOAT(50.0));
	DBG_LOCAL(t_lowY,"lowY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<469>");
	int t_highY=int(this->m_playfieldN->p_CameraY()/FLOAT(40.0)+FLOAT(100.0));
	DBG_LOCAL(t_highY,"highY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<471>");
	int t_randX=0;
	DBG_LOCAL(t_randX,"randX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<472>");
	int t_randY=0;
	DBG_LOCAL(t_randY,"randY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<474>");
	int t_cavesMade=0;
	DBG_LOCAL(t_cavesMade,"cavesMade")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<475>");
	bool t_isReachable=false;
	DBG_LOCAL(t_isReachable,"isReachable")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<477>");
	while(t_cavesMade<t_total){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<478>");
		t_isReachable=false;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<479>");
		t_randX=int(bb_random_Rnd2(Float(t_lowX),Float(t_highX)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<480>");
		t_randY=int(bb_random_Rnd2(Float(t_lowY),Float(t_highY)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<483>");
		if(m_biomes.At(t_randX).At(t_randY)==8 || m_biomes.At(t_randX).At(t_randY)==21 || m_biomes.At(t_randX).At(t_randY)==22){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<486>");
			if(t_randX>0 && m_biomes.At(t_randX-1).At(t_randY)==4){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<487>");
				t_isReachable=true;
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<489>");
			if(t_randX<m_mapWidth-2 && m_biomes.At(t_randX+1).At(t_randY)==4){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<490>");
				t_isReachable=true;
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<492>");
			if(t_randY>0 && m_biomes.At(t_randX).At(t_randY-1)==4){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<493>");
				t_isReachable=true;
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<495>");
			if(t_randY>m_mapHeight-2 && m_biomes.At(t_randX).At(t_randY+1)==4){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<496>");
				t_isReachable=true;
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<500>");
		if(t_isReachable){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<501>");
			bbPrint(String(L"Cave entrance at ",17)+String(t_randX*40)+String(L", ",2)+String(t_randY*40));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<502>");
			m_biomes.At(t_randX).At(t_randY)=18;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<503>");
			gc_assign(m_caves.At(t_cavesMade),(new c_Level)->m_new(t_randX,t_randY,150,100,String(L"Cellular",8)));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<504>");
			this->m_caveX=t_randX;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<505>");
			this->m_caveY=t_randY;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<507>");
			t_cavesMade+=1;
		}
	}
	return 0;
}
int c_NoiseTestScene::p_determineEnemyType(Array<Array<int > > t_enemies,Array<Array<int > > t_habitat){
	DBG_ENTER("NoiseTestScene.determineEnemyType")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_enemies,"enemies")
	DBG_LOCAL(t_habitat,"habitat")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<849>");
	for(int t_i=0;t_i<t_enemies.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<850>");
		for(int t_j=0;t_j<t_enemies.At(0).Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<851>");
			if(!(t_enemies.At(t_i).At(t_j)==-1)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<852>");
				if(t_habitat.At(t_i).At(t_j)==0){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<853>");
					t_enemies.At(t_i).At(t_j)=0;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<854>");
					if(t_habitat.At(t_i).At(t_j)==1){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<855>");
						t_enemies.At(t_i).At(t_j)=1;
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<856>");
						if(t_habitat.At(t_i).At(t_j)==7 || t_habitat.At(t_i).At(t_j)==2 || t_habitat.At(t_i).At(t_j)==5){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<857>");
							t_enemies.At(t_i).At(t_j)=2;
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<858>");
							if(t_habitat.At(t_i).At(t_j)==3 || t_habitat.At(t_i).At(t_j)==31 || t_habitat.At(t_i).At(t_j)==4){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<859>");
								t_enemies.At(t_i).At(t_j)=3;
							}else{
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<860>");
								if(t_habitat.At(t_i).At(t_j)==6){
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<861>");
									t_enemies.At(t_i).At(t_j)=4;
								}else{
									DBG_BLOCK();
									DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<863>");
									t_enemies.At(t_i).At(t_j)=-1;
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
int c_NoiseTestScene::p_placeEnemies(int t_width,int t_height){
	DBG_ENTER("NoiseTestScene.placeEnemies")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<873>");
	gc_assign(this->m_enemyPlacement,bb_noisetestscene_setArray(t_width,t_height));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<874>");
	bb_noisetestscene_randomlyAssignCells(this->m_enemyPlacement,3);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<875>");
	p_determineEnemyType(this->m_enemyPlacement,this->m_biomes);
	return 0;
}
int c_NoiseTestScene::p_OnStart(){
	DBG_ENTER("NoiseTestScene.OnStart")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<168>");
	c_SimplexNoise* t_n=(new c_SimplexNoise)->m_new();
	DBG_LOCAL(t_n,"n")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<169>");
	gc_assign(m_noiseMap,t_n->p_generateOctavedNoiseMap(m_mapWidth,m_mapHeight,5,FLOAT(0.5),FLOAT(1.0)));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<170>");
	gc_assign(m_moisture,t_n->p_generateOctavedNoiseMap(m_mapWidth,m_mapHeight,5,FLOAT(0.5),FLOAT(1.0)));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<171>");
	gc_assign(m_biomes,bb_noisetestscene_setArray(m_mapWidth,m_mapHeight));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<172>");
	p_determineBiomes(m_noiseMap,m_moisture,m_mapWidth,m_mapHeight);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<173>");
	p_processBiomes();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<174>");
	p_detailBiomes();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<176>");
	p_makeRivers(40);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<178>");
	p_selectRandomStartPoint();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<180>");
	p_makeCaves(1);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<181>");
	p_placeEnemies(m_mapWidth,m_mapHeight);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<183>");
	c_AStarSearch* t_overworldSearch=(new c_AStarSearch)->m_new(m_biomes,300,false);
	DBG_LOCAL(t_overworldSearch,"overworldSearch")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<184>");
	c_Path* t_pathToCave=t_overworldSearch->p_findPathOverworld(m_biomes,this->m_startX,this->m_startY,this->m_caveX,this->m_caveY);
	DBG_LOCAL(t_pathToCave,"pathToCave")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<185>");
	if((t_pathToCave)!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<186>");
		c_Enumerator* t_=t_pathToCave->m_steps->p_ObjectEnumerator();
		while(t_->p_HasNext()){
			DBG_BLOCK();
			c_PathStep* t_i=t_->p_NextObject();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<187>");
			if(this->m_biomes.At(t_i->p_getX()).At(t_i->p_getY())==18){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<188>");
				continue;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<189>");
				if(this->m_biomes.At(t_i->p_getX()).At(t_i->p_getY())>11 && this->m_biomes.At(t_i->p_getX()).At(t_i->p_getY())<18){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<190>");
					bbPrint(String(L"MAKING BRIDGE! at: ",19)+String(t_i->p_getX()*40)+String(L", ",2)+String(t_i->p_getY()*40));
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<191>");
					this->m_biomes.At(t_i->p_getX()).At(t_i->p_getY())=1;
				}
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<195>");
		bbPrint(String(L"No path",7));
	}
	return 0;
}
int c_NoiseTestScene::p_OnStop(){
	DBG_ENTER("NoiseTestScene.OnStop")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
bool c_NoiseTestScene::p_isWalkable(int t_tile){
	DBG_ENTER("NoiseTestScene.isWalkable")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_tile,"tile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<879>");
	if(t_tile==0 || t_tile>7 && t_tile<18 || t_tile>20 && t_tile<31){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<881>");
		return false;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<883>");
		return true;
	}
}
int c_NoiseTestScene::p_checkCameraBounds(){
	DBG_ENTER("NoiseTestScene.checkCameraBounds")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<636>");
	if(this->m_playfieldN->p_CameraX()<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<637>");
		this->m_playfieldN->p_CameraX2(FLOAT(0.0));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<639>");
	if(this->m_playfieldN->p_CameraY()<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<640>");
		this->m_playfieldN->p_CameraY2(FLOAT(0.0));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<643>");
	if(this->m_playfieldN->p_CameraX()+this->m_playfieldN->p_Width()>this->m_playfieldN->p_Width()*FLOAT(40.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<644>");
		this->m_playfieldN->p_CameraX2(this->m_playfieldN->p_Width()*FLOAT(39.0));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<646>");
	if(this->m_playfieldN->p_CameraY()+this->m_playfieldN->p_Height()>this->m_playfieldN->p_Height()*FLOAT(40.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<647>");
		this->m_playfieldN->p_CameraY2(this->m_playfieldN->p_Height()*FLOAT(39.0));
	}
	return 0;
}
int c_NoiseTestScene::p_OnUpdate(){
	DBG_ENTER("NoiseTestScene.OnUpdate")
	c_NoiseTestScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<547>");
	if((bb_input_KeyDown(17))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<549>");
		if((bb_input_KeyDown(65))!=0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<550>");
			this->m_playfieldN->p_AlphaFade2(this->m_playfieldN->p_AlphaFade()-FLOAT(0.01));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<551>");
			if(this->m_playfieldN->p_AlphaFade()<FLOAT(0.0)){
				DBG_BLOCK();
				this->m_playfieldN->p_AlphaFade2(this->m_playfieldN->p_AlphaFade()+FLOAT(1.0));
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<554>");
		if((bb_input_KeyDown(90))!=0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<555>");
			this->m_playfieldN->p_ZoomX2(this->m_playfieldN->p_ZoomX()-FLOAT(0.01));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<556>");
			this->m_playfieldN->p_ZoomY2(this->m_playfieldN->p_ZoomY()-FLOAT(0.01));
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<561>");
		if((bb_input_KeyDown(65))!=0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<562>");
			this->m_playfieldN->p_AlphaFade2(this->m_playfieldN->p_AlphaFade()+FLOAT(0.01));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<563>");
			if(this->m_playfieldN->p_AlphaFade()>FLOAT(1.0)){
				DBG_BLOCK();
				this->m_playfieldN->p_AlphaFade2(this->m_playfieldN->p_AlphaFade()-FLOAT(1.0));
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<566>");
		if((bb_input_KeyDown(90))!=0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<567>");
			this->m_playfieldN->p_ZoomX2(this->m_playfieldN->p_ZoomX()+FLOAT(0.01));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<568>");
			this->m_playfieldN->p_ZoomY2(this->m_playfieldN->p_ZoomY()+FLOAT(0.01));
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<588>");
	if((bb_input_KeyDown(37))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<589>");
		if(p_isWalkable(m_biomes.At((m_bBox->m_x-5)/40).At(m_bBox->m_y/40)) && p_isWalkable(m_biomes.At((m_bBox->m_x-5)/40).At((m_bBox->m_y+m_bBox->m_height)/40))){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<590>");
			if(m_sprite1->p_PositionX()-this->m_playfieldN->p_CameraX()<FLOAT(200.0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<591>");
				this->m_playfieldN->p_CameraX2(this->m_playfieldN->p_CameraX()-FLOAT(5.0));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<594>");
			m_sprite1->p_PositionX2(m_sprite1->p_PositionX()-FLOAT(5.0));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<595>");
			this->m_bBox->m_x-=5;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<599>");
	if((bb_input_KeyDown(39))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<600>");
		if(p_isWalkable(m_biomes.At((m_bBox->m_x+m_bBox->m_width+5)/40).At(m_bBox->m_y/40)) && p_isWalkable(m_biomes.At((m_bBox->m_x+m_bBox->m_width+5)/40).At((m_bBox->m_y+m_bBox->m_height)/40))){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<601>");
			if(m_sprite1->p_PositionX()-this->m_playfieldN->p_CameraX()>FLOAT(350.0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<602>");
				this->m_playfieldN->p_CameraX2(this->m_playfieldN->p_CameraX()+FLOAT(5.0));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<604>");
			m_sprite1->p_PositionX2(m_sprite1->p_PositionX()+FLOAT(5.0));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<605>");
			this->m_bBox->m_x+=5;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<609>");
	if((bb_input_KeyDown(38))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<610>");
		if(p_isWalkable(m_biomes.At(m_bBox->m_x/40).At((m_bBox->m_y-5)/40)) && p_isWalkable(m_biomes.At((m_bBox->m_x+m_bBox->m_width)/40).At((m_bBox->m_y-5)/40))){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<611>");
			if(m_sprite1->p_PositionY()-this->m_playfieldN->p_CameraY()<FLOAT(200.0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<612>");
				this->m_playfieldN->p_CameraY2(this->m_playfieldN->p_CameraY()-FLOAT(5.0));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<614>");
			m_sprite1->p_PositionY2(m_sprite1->p_PositionY()-FLOAT(5.0));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<615>");
			this->m_bBox->m_y-=5;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<619>");
	if((bb_input_KeyDown(40))!=0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<620>");
		if(p_isWalkable(m_biomes.At(m_bBox->m_x/40).At((m_bBox->m_y+m_bBox->m_height+5)/40)) && p_isWalkable(m_biomes.At((m_bBox->m_x+m_bBox->m_width)/40).At((m_bBox->m_y+m_bBox->m_height+5)/40))){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<621>");
			if(m_sprite1->p_PositionY()-this->m_playfieldN->p_CameraY()>FLOAT(250.0)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<622>");
				this->m_playfieldN->p_CameraY2(this->m_playfieldN->p_CameraY()+FLOAT(5.0));
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<625>");
			m_sprite1->p_PositionY2(m_sprite1->p_PositionY()+FLOAT(5.0));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<626>");
			this->m_bBox->m_y+=5;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<629>");
	p_checkCameraBounds();
	return 0;
}
void c_NoiseTestScene::mark(){
	c_iEngine::mark();
	gc_mark_q(m_playfieldN);
	gc_mark_q(m_layer);
	gc_mark_q(m_sprite1);
	gc_mark_q(m_deepWaterTiles);
	gc_mark_q(m_shallowWaterTiles);
	gc_mark_q(m_beachTiles);
	gc_mark_q(m_lightGrassTiles);
	gc_mark_q(m_heavyGrassTiles);
	gc_mark_q(m_swampTiles);
	gc_mark_q(m_forestTiles);
	gc_mark_q(m_desertTiles);
	gc_mark_q(m_mountainTiles);
	gc_mark_q(m_darkSnowMountainTiles);
	gc_mark_q(m_lightSnowMountainTiles);
	gc_mark_q(m_riverTiles);
	gc_mark_q(m_caveEntranceTiles);
	gc_mark_q(m_biomes);
	gc_mark_q(m_enemyPlacement);
	gc_mark_q(m_noiseMap);
	gc_mark_q(m_moisture);
	gc_mark_q(m_bBox);
	gc_mark_q(m_caves);
}
String c_NoiseTestScene::debug(){
	String t="(NoiseTestScene)\n";
	t=c_iEngine::debug()+t;
	t+=dbg_decl("deepWaterTiles",&m_deepWaterTiles);
	t+=dbg_decl("shallowWaterTiles",&m_shallowWaterTiles);
	t+=dbg_decl("beachTiles",&m_beachTiles);
	t+=dbg_decl("lightGrassTiles",&m_lightGrassTiles);
	t+=dbg_decl("heavyGrassTiles",&m_heavyGrassTiles);
	t+=dbg_decl("swampTiles",&m_swampTiles);
	t+=dbg_decl("forestTiles",&m_forestTiles);
	t+=dbg_decl("desertTiles",&m_desertTiles);
	t+=dbg_decl("mountainTiles",&m_mountainTiles);
	t+=dbg_decl("darkSnowMountainTiles",&m_darkSnowMountainTiles);
	t+=dbg_decl("lightSnowMountainTiles",&m_lightSnowMountainTiles);
	t+=dbg_decl("riverTiles",&m_riverTiles);
	t+=dbg_decl("caveEntranceTiles",&m_caveEntranceTiles);
	t+=dbg_decl("noiseMap",&m_noiseMap);
	t+=dbg_decl("moisture",&m_moisture);
	t+=dbg_decl("biomes",&m_biomes);
	t+=dbg_decl("caves",&m_caves);
	t+=dbg_decl("enemyPlacement",&m_enemyPlacement);
	t+=dbg_decl("mapWidth",&m_mapWidth);
	t+=dbg_decl("mapHeight",&m_mapHeight);
	t+=dbg_decl("playfieldN",&m_playfieldN);
	t+=dbg_decl("sprite1",&m_sprite1);
	t+=dbg_decl("layer",&m_layer);
	t+=dbg_decl("bBox",&m_bBox);
	t+=dbg_decl("caveX",&m_caveX);
	t+=dbg_decl("caveY",&m_caveY);
	t+=dbg_decl("startX",&m_startX);
	t+=dbg_decl("startY",&m_startY);
	t+=dbg_decl("tileCounter",&m_tileCounter);
	return t;
}
c_NoiseTestScene* bb_main_noiseTest;
c_Path::c_Path(){
	m_steps=0;
}
c_Path* c_Path::m_new(){
	DBG_ENTER("Path.new")
	c_Path *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<423>");
	gc_assign(this->m_steps,(new c_List)->m_new());
	return this;
}
int c_Path::p_prependStep(int t_xCoord,int t_yCoord){
	DBG_ENTER("Path.prependStep")
	c_Path *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_xCoord,"xCoord")
	DBG_LOCAL(t_yCoord,"yCoord")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<437>");
	m_steps->p_AddFirst((new c_PathStep)->m_new(t_xCoord,t_yCoord));
	return 0;
}
int c_Path::p_pathLength(){
	DBG_ENTER("Path.pathLength")
	c_Path *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<429>");
	int t_=m_steps->p_Count();
	return t_;
}
void c_Path::mark(){
	Object::mark();
	gc_mark_q(m_steps);
}
String c_Path::debug(){
	String t="(Path)\n";
	t+=dbg_decl("steps",&m_steps);
	return t;
}
c_PathStep::c_PathStep(){
	m_x=0;
	m_y=0;
}
c_PathStep* c_PathStep::m_new(int t_theX,int t_theY){
	DBG_ENTER("PathStep.new")
	c_PathStep *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_theX,"theX")
	DBG_LOCAL(t_theY,"theY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<457>");
	this->m_x=t_theX;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<458>");
	this->m_y=t_theY;
	return this;
}
c_PathStep* c_PathStep::m_new2(){
	DBG_ENTER("PathStep.new")
	c_PathStep *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<452>");
	return this;
}
int c_PathStep::p_getX(){
	DBG_ENTER("PathStep.getX")
	c_PathStep *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<462>");
	return this->m_x;
}
int c_PathStep::p_getY(){
	DBG_ENTER("PathStep.getY")
	c_PathStep *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<466>");
	return this->m_y;
}
void c_PathStep::mark(){
	Object::mark();
}
String c_PathStep::debug(){
	String t="(PathStep)\n";
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	return t;
}
c_List::c_List(){
	m__head=((new c_HeadNode)->m_new());
}
c_List* c_List::m_new(){
	DBG_ENTER("List.new")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
c_Node2* c_List::p_AddLast(c_PathStep* t_data){
	DBG_ENTER("List.AddLast")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<108>");
	c_Node2* t_=(new c_Node2)->m_new(m__head,m__head->m__pred,t_data);
	return t_;
}
c_List* c_List::m_new2(Array<c_PathStep* > t_data){
	DBG_ENTER("List.new")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<13>");
	Array<c_PathStep* > t_=t_data;
	int t_2=0;
	while(t_2<t_.Length()){
		DBG_BLOCK();
		c_PathStep* t_t=t_.At(t_2);
		t_2=t_2+1;
		DBG_LOCAL(t_t,"t")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<14>");
		p_AddLast(t_t);
	}
	return this;
}
c_Node2* c_List::p_AddFirst(c_PathStep* t_data){
	DBG_ENTER("List.AddFirst")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<104>");
	c_Node2* t_=(new c_Node2)->m_new(m__head->m__succ,m__head,t_data);
	return t_;
}
int c_List::p_Count(){
	DBG_ENTER("List.Count")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<41>");
	int t_n=0;
	c_Node2* t_node=m__head->m__succ;
	DBG_LOCAL(t_n,"n")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<42>");
	while(t_node!=m__head){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<43>");
		t_node=t_node->m__succ;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<44>");
		t_n+=1;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<46>");
	return t_n;
}
c_Enumerator* c_List::p_ObjectEnumerator(){
	DBG_ENTER("List.ObjectEnumerator")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<186>");
	c_Enumerator* t_=(new c_Enumerator)->m_new(this);
	return t_;
}
void c_List::mark(){
	Object::mark();
	gc_mark_q(m__head);
}
String c_List::debug(){
	String t="(List)\n";
	t+=dbg_decl("_head",&m__head);
	return t;
}
c_Node2::c_Node2(){
	m__succ=0;
	m__pred=0;
	m__data=0;
}
c_Node2* c_Node2::m_new(c_Node2* t_succ,c_Node2* t_pred,c_PathStep* t_data){
	DBG_ENTER("Node.new")
	c_Node2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_succ,"succ")
	DBG_LOCAL(t_pred,"pred")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<261>");
	gc_assign(m__succ,t_succ);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<262>");
	gc_assign(m__pred,t_pred);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<263>");
	gc_assign(m__succ->m__pred,this);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<264>");
	gc_assign(m__pred->m__succ,this);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<265>");
	gc_assign(m__data,t_data);
	return this;
}
c_Node2* c_Node2::m_new2(){
	DBG_ENTER("Node.new")
	c_Node2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<258>");
	return this;
}
void c_Node2::mark(){
	Object::mark();
	gc_mark_q(m__succ);
	gc_mark_q(m__pred);
	gc_mark_q(m__data);
}
String c_Node2::debug(){
	String t="(Node)\n";
	t+=dbg_decl("_succ",&m__succ);
	t+=dbg_decl("_pred",&m__pred);
	t+=dbg_decl("_data",&m__data);
	return t;
}
c_HeadNode::c_HeadNode(){
}
c_HeadNode* c_HeadNode::m_new(){
	DBG_ENTER("HeadNode.new")
	c_HeadNode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<310>");
	c_Node2::m_new2();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<311>");
	gc_assign(m__succ,(this));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<312>");
	gc_assign(m__pred,(this));
	return this;
}
void c_HeadNode::mark(){
	c_Node2::mark();
}
String c_HeadNode::debug(){
	String t="(HeadNode)\n";
	t=c_Node2::debug()+t;
	return t;
}
int bb_app__updateRate;
int bb_app_UpdateRate(){
	DBG_ENTER("UpdateRate")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<229>");
	return bb_app__updateRate;
}
void bb_app_SetUpdateRate(int t_hertz){
	DBG_ENTER("SetUpdateRate")
	DBG_LOCAL(t_hertz,"hertz")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<224>");
	bb_app__updateRate=t_hertz;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<225>");
	bb_app__game->SetUpdateRate(t_hertz);
}
void bb_app2_iStart(int t_hertz){
	DBG_ENTER("iStart")
	DBG_LOCAL(t_hertz,"hertz")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<213>");
	if(t_hertz!=bb_app_UpdateRate()){
		DBG_BLOCK();
		bb_app_SetUpdateRate(t_hertz);
	}
}
void bb_app2_iStart2(c_iScene* t_scene){
	DBG_ENTER("iStart")
	DBG_LOCAL(t_scene,"scene")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<220>");
	gc_assign(bb_app2_iNextScene,t_scene);
}
void bb_app2_iStart3(c_iScene* t_scene,int t_hertz){
	DBG_ENTER("iStart")
	DBG_LOCAL(t_scene,"scene")
	DBG_LOCAL(t_hertz,"hertz")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<228>");
	gc_assign(bb_app2_iNextScene,t_scene);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<230>");
	if(t_hertz!=bb_app_UpdateRate()){
		DBG_BLOCK();
		bb_app_SetUpdateRate(t_hertz);
	}
}
c_iEngine* bb_globals_iEnginePointer;
int bb_functions_iTimeCode(){
	DBG_ENTER("iTimeCode")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<434>");
	return bb_globals_iEnginePointer->m__timeCode;
}
Array<Float > bb_graphics_GetScissor(){
	DBG_ENTER("GetScissor")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<297>");
	Float t_2[]={bb_graphics_context->m_scissor_x,bb_graphics_context->m_scissor_y,bb_graphics_context->m_scissor_width,bb_graphics_context->m_scissor_height};
	Array<Float > t_=Array<Float >(t_2,4);
	return t_;
}
int bb_graphics_GetScissor2(Array<Float > t_scissor){
	DBG_ENTER("GetScissor")
	DBG_LOCAL(t_scissor,"scissor")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<301>");
	t_scissor.At(0)=bb_graphics_context->m_scissor_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<302>");
	t_scissor.At(1)=bb_graphics_context->m_scissor_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<303>");
	t_scissor.At(2)=bb_graphics_context->m_scissor_width;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<304>");
	t_scissor.At(3)=bb_graphics_context->m_scissor_height;
	return 0;
}
int bb_graphics_DebugRenderDevice(){
	DBG_ENTER("DebugRenderDevice")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<53>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<378>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<380>");
	bb_graphics_renderDevice->Cls(t_r,t_g,t_b);
	return 0;
}
void bb_gfx_iCls(Float t_red,Float t_green,Float t_blue){
	DBG_ENTER("iCls")
	DBG_LOCAL(t_red,"red")
	DBG_LOCAL(t_green,"green")
	DBG_LOCAL(t_blue,"blue")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<19>");
	Array<Float > t_s=bb_graphics_GetScissor();
	DBG_LOCAL(t_s,"s")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<21>");
	bb_graphics_SetScissor(FLOAT(0.0),FLOAT(0.0),Float(bb_app_DeviceWidth()),Float(bb_app_DeviceHeight()));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<22>");
	bb_graphics_Cls(t_red,t_green,t_blue);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<24>");
	bb_graphics_SetScissor(t_s.At(0),t_s.At(1),t_s.At(2),t_s.At(3));
}
c_iEngineObject::c_iEngineObject(){
	m__autoDestroy=false;
}
c_iEngineObject* c_iEngineObject::m_new(){
	DBG_ENTER("iEngineObject.new")
	c_iEngineObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<10>");
	c_iObject::m_new();
	return this;
}
int c_iEngineObject::p_OnRemove(){
	DBG_ENTER("iEngineObject.OnRemove")
	c_iEngineObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<72>");
	return 0;
}
void c_iEngineObject::p_Remove(){
	DBG_ENTER("iEngineObject.Remove")
	c_iEngineObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<79>");
	p_OnRemove();
}
c_iList6* c_iEngineObject::m_destroyList;
int c_iEngineObject::p_OnDestroy(){
	DBG_ENTER("iEngineObject.OnDestroy")
	c_iEngineObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<65>");
	return 0;
}
void c_iEngineObject::p_Destroy(){
	DBG_ENTER("iEngineObject.Destroy")
	c_iEngineObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<38>");
	p_OnDestroy();
}
void c_iEngineObject::m_DestroyList2(){
	DBG_ENTER("iEngineObject.DestroyList")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<46>");
	if(m_destroyList->p_Length()==0){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<48>");
	c_iEngineObject* t_o=m_destroyList->p_First();
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<49>");
	while((t_o)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<51>");
		m_destroyList->p_Remove6(t_o);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<53>");
		if(t_o->m__inPool==0){
			DBG_BLOCK();
			t_o->p_Destroy();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<55>");
		t_o=m_destroyList->p_Ascend();
	}
}
void c_iEngineObject::mark(){
	c_iObject::mark();
}
String c_iEngineObject::debug(){
	String t="(iEngineObject)\n";
	t=c_iObject::debug()+t;
	t+=dbg_decl("destroyList",&c_iEngineObject::m_destroyList);
	t+=dbg_decl("_autoDestroy",&m__autoDestroy);
	return t;
}
c_iPlayfield::c_iPlayfield(){
	m__alphaFade=FLOAT(1.0);
	m__colorFade=FLOAT(1.0);
	m__visible=true;
	m__x=FLOAT(.0);
	m__y=FLOAT(.0);
	m__vwidth=0;
	m__scaleX=FLOAT(1.0);
	m__vheight=0;
	m__scaleY=FLOAT(1.0);
	m__autoCls=true;
	m__clsRed=FLOAT(.0);
	m__clsGreen=FLOAT(.0);
	m__clsBlue=FLOAT(.0);
	m__zoomPointX=FLOAT(.0);
	m__zoomPointY=FLOAT(.0);
	m__zoomX=FLOAT(1.0);
	m__zoomY=FLOAT(1.0);
	m__rotation=FLOAT(.0);
	m__backgroundRender=false;
	m__cameraX=FLOAT(.0);
	m__cameraY=FLOAT(.0);
	m__layerList=(new c_iList2)->m_new();
	m__currentLayer=0;
	m__height=0;
	m__width=0;
	m__guiEnabled=true;
	m__guiList=Array<c_iList4* >(1);
	m__guiPage=0;
	m__guiLastObject=0;
	m__nextGuiPage=-1;
	m__z=FLOAT(.0);
}
int c_iPlayfield::p_SetAlpha(Float t_alpha){
	DBG_ENTER("iPlayfield.SetAlpha")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1255>");
	bb_graphics_SetAlpha(t_alpha*bb_globals_iEnginePointer->m__alphaFade*m__alphaFade);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1257>");
	return 0;
}
int c_iPlayfield::p_SetColor(Float t_red,Float t_green,Float t_blue){
	DBG_ENTER("iPlayfield.SetColor")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_red,"red")
	DBG_LOCAL(t_green,"green")
	DBG_LOCAL(t_blue,"blue")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1267>");
	bb_graphics_SetColor(t_red*bb_globals_iEnginePointer->m__colorFade*m__colorFade,t_green*bb_globals_iEnginePointer->m__colorFade*m__colorFade,t_blue*bb_globals_iEnginePointer->m__colorFade*m__colorFade);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1269>");
	return 0;
}
int c_iPlayfield::p_OnBackgroundRender(){
	DBG_ENTER("iPlayfield.OnBackgroundRender")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<728>");
	return 0;
}
int c_iPlayfield::p_OnRender(){
	DBG_ENTER("iPlayfield.OnRender")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<748>");
	return 0;
}
Float c_iPlayfield::p_PositionX(){
	DBG_ENTER("iPlayfield.PositionX")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<784>");
	return m__x;
}
void c_iPlayfield::p_PositionX2(Float t_x){
	DBG_ENTER("iPlayfield.PositionX")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<791>");
	m__x=t_x;
}
Float c_iPlayfield::p_CameraX(){
	DBG_ENTER("iPlayfield.CameraX")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<278>");
	return m__cameraX;
}
void c_iPlayfield::p_CameraX2(Float t_x){
	DBG_ENTER("iPlayfield.CameraX")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<285>");
	m__cameraX=t_x;
}
Float c_iPlayfield::p_PositionY(){
	DBG_ENTER("iPlayfield.PositionY")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<798>");
	return m__y;
}
void c_iPlayfield::p_PositionY2(Float t_y){
	DBG_ENTER("iPlayfield.PositionY")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<805>");
	m__y=t_y;
}
Float c_iPlayfield::p_CameraY(){
	DBG_ENTER("iPlayfield.CameraY")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<292>");
	return m__cameraY;
}
void c_iPlayfield::p_CameraY2(Float t_y){
	DBG_ENTER("iPlayfield.CameraY")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<299>");
	m__cameraY=t_y;
}
Float c_iPlayfield::p_VWidth(){
	DBG_ENTER("iPlayfield.VWidth")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1765>");
	Float t_=Float(m__vwidth);
	return t_;
}
Float c_iPlayfield::p_ZoomPointX(){
	DBG_ENTER("iPlayfield.ZoomPointX")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1889>");
	return m__zoomPointX;
}
void c_iPlayfield::p_ZoomPointX2(Float t_zoomPointX){
	DBG_ENTER("iPlayfield.ZoomPointX")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_zoomPointX,"zoomPointX")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1931>");
	m__zoomPointX=t_zoomPointX;
}
Float c_iPlayfield::p_VHeight(){
	DBG_ENTER("iPlayfield.VHeight")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1681>");
	Float t_=Float(m__vheight);
	return t_;
}
Float c_iPlayfield::p_ZoomPointY(){
	DBG_ENTER("iPlayfield.ZoomPointY")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1896>");
	return m__zoomPointY;
}
void c_iPlayfield::p_ZoomPointY2(Float t_zoomPointY){
	DBG_ENTER("iPlayfield.ZoomPointY")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_zoomPointY,"zoomPointY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1938>");
	m__zoomPointY=t_zoomPointY;
}
void c_iPlayfield::p_ZoomPoint(Float t_zoomPointX,Float t_zoomPointY){
	DBG_ENTER("iPlayfield.ZoomPoint")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_zoomPointX,"zoomPointX")
	DBG_LOCAL(t_zoomPointY,"zoomPointY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1880>");
	p_ZoomPointX2(t_zoomPointX);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1881>");
	p_ZoomPointY2(t_zoomPointY);
}
void c_iPlayfield::p_VHeight2(int t_height){
	DBG_ENTER("iPlayfield.VHeight")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1695>");
	Float t_h=Float(bb_app_DeviceHeight());
	DBG_LOCAL(t_h,"h")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1697>");
	Float t_s=t_h/Float(t_height);
	DBG_LOCAL(t_s,"s")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1699>");
	m__vwidth=int(Float(m__width)/t_s);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1701>");
	m__vheight=t_height;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1703>");
	m__scaleX=t_s;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1704>");
	m__scaleY=t_s;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1706>");
	p_ZoomPoint2();
}
void c_iPlayfield::p_ZoomPoint2(){
	DBG_ENTER("iPlayfield.ZoomPoint")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1870>");
	p_ZoomPointX2(p_VWidth()*FLOAT(.5));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1871>");
	p_ZoomPointY2(p_VHeight()*FLOAT(.5));
}
void c_iPlayfield::p_VWidth2(int t_width){
	DBG_ENTER("iPlayfield.VWidth")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1779>");
	Float t_w=Float(bb_app_DeviceWidth());
	DBG_LOCAL(t_w,"w")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1781>");
	Float t_s=t_w/Float(t_width);
	DBG_LOCAL(t_s,"s")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1783>");
	m__vwidth=t_width;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1785>");
	m__vheight=int(Float(m__height)/t_s);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1787>");
	m__scaleX=t_s;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1788>");
	m__scaleY=t_s;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1790>");
	p_ZoomPoint2();
}
int c_iPlayfield::p_OnTopRender(){
	DBG_ENTER("iPlayfield.OnTopRender")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<752>");
	return 0;
}
void c_iPlayfield::p_RenderGui(){
	DBG_ENTER("iPlayfield.RenderGui")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1023>");
	c_iStack2D::m_Push();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1025>");
	if(bb_globals_iEnginePointer->m__borders){
		DBG_BLOCK();
		bb_gfx_iSetScissor(m__x,m__y,Float(m__vwidth)*m__scaleY,Float(m__vheight)*m__scaleY);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1028>");
	bb_graphics_Scale(m__scaleX,m__scaleY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1030>");
	bb_graphics_Translate(m__x/m__scaleX,m__y/m__scaleY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1032>");
	c_iGuiObject* t_g=m__guiList.At(m__guiPage)->p_First();
	DBG_LOCAL(t_g,"g")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1033>");
	while((t_g)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1035>");
		if(t_g->m__enabled==1 && t_g->m__visible){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1036>");
			if(t_g->m__ghost){
				DBG_BLOCK();
				t_g->p_RenderGhost();
			}else{
				DBG_BLOCK();
				t_g->p_Render();
			}
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1039>");
		t_g=m__guiList.At(m__guiPage)->p_Ascend();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1043>");
	if(((m__guiLastObject)!=0) && m__guiLastObject->m__enabled==1 && m__guiLastObject->m__visible){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1045>");
		bb_graphics_SetAlpha(FLOAT(.75));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1046>");
		bb_graphics_SetBlend(0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1047>");
		bb_graphics_SetColor(FLOAT(1.0),FLOAT(1.0),FLOAT(1.0));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1048>");
		bb_graphics_Rotate(FLOAT(0.0));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1049>");
		bb_graphics_DrawRect(FLOAT(0.0),FLOAT(0.0),Float(m__vwidth),Float(m__vheight));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1051>");
		if(m__guiLastObject->m__ghost){
			DBG_BLOCK();
			m__guiLastObject->p_RenderGhost();
		}else{
			DBG_BLOCK();
			m__guiLastObject->p_Render();
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1055>");
	c_iStack2D::m_Pop();
}
void c_iPlayfield::p_Render(){
	DBG_ENTER("iPlayfield.Render")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<933>");
	c_iStack2D::m_Push();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<935>");
	if(bb_globals_iEnginePointer->m__borders){
		DBG_BLOCK();
		bb_gfx_iSetScissor(m__x,m__y,Float(m__vwidth)*m__scaleX,Float(m__vheight)*m__scaleY);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<938>");
	if(m__autoCls){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<939>");
		bb_graphics_Cls(m__clsRed*bb_globals_iEnginePointer->m__colorFade*m__colorFade,m__clsGreen*bb_globals_iEnginePointer->m__colorFade*m__colorFade,m__clsBlue*bb_globals_iEnginePointer->m__colorFade*m__colorFade);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<942>");
	bb_graphics_Scale(m__scaleX,m__scaleY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<944>");
	bb_graphics_Translate(m__x/m__scaleX+m__zoomPointX,m__y/m__scaleY+m__zoomPointY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<945>");
	bb_graphics_Scale(m__zoomX,m__zoomY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<946>");
	bb_graphics_Translate(-(m__x/m__scaleX+m__zoomPointX),-(m__y/m__scaleY+m__zoomPointY));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<948>");
	bb_graphics_Translate(m__x/m__scaleX+m__zoomPointX,m__y/m__scaleY+m__zoomPointY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<949>");
	bb_graphics_Rotate(m__rotation*Float(bb_globals_iRotation));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<950>");
	bb_graphics_Translate(-(m__x/m__scaleX+m__zoomPointX),-(m__y/m__scaleY+m__zoomPointY));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<952>");
	if(m__backgroundRender){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<953>");
		c_iStack2D::m_Push();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<954>");
		p_OnBackgroundRender();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<955>");
		c_iStack2D::m_Pop();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<958>");
	c_iStack2D::m_Push();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<960>");
	bb_graphics_Translate(m__x/m__scaleX-m__cameraX,m__y/m__scaleY-m__cameraY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<962>");
	if(bb_globals_iEnginePointer->m__renderToPlayfield==true){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<964>");
		c_iStack2D::m_Push();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<966>");
		bb_globals_iEnginePointer->p_SetAlpha(FLOAT(1.0));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<967>");
		bb_globals_iEnginePointer->p_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<969>");
		bb_globals_iEnginePointer->p_OnRender();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<971>");
		c_iStack2D::m_Pop();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<975>");
	c_iStack2D::m_Push();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<976>");
	p_OnRender();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<977>");
	c_iStack2D::m_Pop();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<979>");
	c_iStack2D::m_Pop();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<981>");
	gc_assign(m__currentLayer,m__layerList->p_First());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<983>");
	while((m__currentLayer)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<985>");
		if(((m__currentLayer->m__enabled)!=0) && m__currentLayer->m__visible){
			DBG_BLOCK();
			m__currentLayer->p_Render();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<987>");
		gc_assign(m__currentLayer,m__layerList->p_Ascend());
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<991>");
	c_iStack2D::m_Push();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<993>");
	bb_graphics_Translate(m__x/m__scaleX-m__cameraX,m__y/m__scaleY-m__cameraY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<995>");
	if(bb_globals_iEnginePointer->m__renderToPlayfield==true){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<997>");
		c_iStack2D::m_Push();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<999>");
		bb_globals_iEnginePointer->p_SetAlpha(FLOAT(1.0));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1000>");
		bb_globals_iEnginePointer->p_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1002>");
		bb_globals_iEnginePointer->p_OnTopRender();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1004>");
		c_iStack2D::m_Pop();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1008>");
	p_OnTopRender();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1010>");
	c_iStack2D::m_Pop();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1012>");
	c_iStack2D::m_Pop();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1014>");
	if(m__guiEnabled){
		DBG_BLOCK();
		p_RenderGui();
	}
}
int c_iPlayfield::p_GuiPage(){
	DBG_ENTER("iPlayfield.GuiPage")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<478>");
	if(m__nextGuiPage!=-1){
		DBG_BLOCK();
		return m__nextGuiPage;
	}else{
		DBG_BLOCK();
		return m__guiPage;
	}
}
void c_iPlayfield::p_InitGuiPages(int t_pages){
	DBG_ENTER("iPlayfield.InitGuiPages")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_pages,"pages")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<537>");
	if(m__guiList.Length()<=t_pages){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<539>");
		gc_assign(m__guiList,m__guiList.Resize(t_pages+1));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<541>");
		for(int t_p=0;t_p<m__guiList.Length();t_p=t_p+1){
			DBG_BLOCK();
			DBG_LOCAL(t_p,"p")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<542>");
			if(!((m__guiList.At(t_p))!=0)){
				DBG_BLOCK();
				gc_assign(m__guiList.At(t_p),(new c_iList4)->m_new());
			}
		}
	}
}
int c_iPlayfield::p_OnGuiPageChange2(){
	DBG_ENTER("iPlayfield.OnGuiPageChange")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<744>");
	return 0;
}
void c_iPlayfield::p_GuiPage2(int t_page){
	DBG_ENTER("iPlayfield.GuiPage")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_page,"page")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<486>");
	p_InitGuiPages(t_page);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<488>");
	if(m__guiPage!=t_page){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<489>");
		p_OnGuiPageChange2();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<490>");
		bb_globals_iEnginePointer->p_OnGuiPageChange(this);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<493>");
	m__nextGuiPage=t_page;
}
void c_iPlayfield::p_Set2(){
	DBG_ENTER("iPlayfield.Set")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1247>");
	if((bb_globals_iEnginePointer)!=0){
		DBG_BLOCK();
		gc_assign(bb_globals_iEnginePointer->m__playfieldPointer,this);
	}
}
int c_iPlayfield::p_OnCreate(){
	DBG_ENTER("iPlayfield.OnCreate")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<737>");
	return 0;
}
void c_iPlayfield::p_SystemInit(){
	DBG_ENTER("iPlayfield.SystemInit")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1504>");
	p_Set2();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1506>");
	m__height=bb_app_DeviceHeight();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1507>");
	m__width=bb_app_DeviceWidth();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1509>");
	m__vheight=bb_app_DeviceHeight();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1510>");
	m__vwidth=bb_app_DeviceWidth();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1512>");
	gc_assign(m__guiList.At(0),(new c_iList4)->m_new());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1514>");
	p_ZoomPoint2();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1516>");
	p_OnCreate();
}
c_iPlayfield* c_iPlayfield::m_new(){
	DBG_ENTER("iPlayfield.new")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<693>");
	c_iEngineObject::m_new();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<694>");
	p_SystemInit();
	return this;
}
c_iPlayfield* c_iPlayfield::m_new2(int t_width,int t_height){
	DBG_ENTER("iPlayfield.new")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<700>");
	c_iEngineObject::m_new();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<702>");
	p_SystemInit();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<704>");
	m__vwidth=t_width;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<705>");
	m__vheight=t_height;
	return this;
}
c_iPlayfield* c_iPlayfield::m_new3(Float t_x,Float t_y,int t_width,int t_height){
	DBG_ENTER("iPlayfield.new")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<712>");
	c_iEngineObject::m_new();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<714>");
	p_SystemInit();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<716>");
	m__x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<717>");
	m__y=t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<719>");
	m__vwidth=t_width;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<720>");
	m__vheight=t_height;
	return this;
}
void c_iPlayfield::p_UpdateGui(){
	DBG_ENTER("iPlayfield.UpdateGui")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1586>");
	if(m__nextGuiPage>=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1587>");
		m__guiPage=m__nextGuiPage;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1588>");
		m__nextGuiPage=-1;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1591>");
	if((m__guiLastObject)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1593>");
		m__guiLastObject->p_UpdateWorldXY();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1595>");
		if(m__guiLastObject->m__enabled==1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1597>");
			if(m__guiLastObject->m__ghost){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1599>");
				m__guiLastObject->p_UpdateGhost();
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1603>");
				m__guiLastObject->p_UpdateInput();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1604>");
				m__guiLastObject->p_Update();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1606>");
				if((m__guiLastObject)!=0){
					DBG_BLOCK();
					m__guiLastObject->p_OnUpdate();
				}
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1614>");
		c_iGuiObject* t_g=0;
		DBG_LOCAL(t_g,"g")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1616>");
		t_g=m__guiList.At(m__guiPage)->p_Last();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1618>");
		while((t_g)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1620>");
			t_g->p_UpdateWorldXY();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1622>");
			if(t_g->m__enabled==1 && !t_g->m__ghost){
				DBG_BLOCK();
				t_g->p_UpdateInput();
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1624>");
			t_g=m__guiList.At(m__guiPage)->p_Descend();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1628>");
		t_g=m__guiList.At(m__guiPage)->p_First();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1629>");
		while((t_g)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1631>");
			if(t_g->m__enabled==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1633>");
				if(t_g->m__ghost){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1635>");
					t_g->p_UpdateGhost();
				}else{
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1639>");
					t_g->p_Update();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1640>");
					t_g->p_OnUpdate();
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1645>");
				if(t_g->m__enabled>1){
					DBG_BLOCK();
					t_g->m__enabled=t_g->m__enabled-1;
				}
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1648>");
			t_g=m__guiList.At(m__guiPage)->p_Ascend();
		}
	}
}
int c_iPlayfield::p_OnUpdate(){
	DBG_ENTER("iPlayfield.OnUpdate")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<756>");
	return 0;
}
void c_iPlayfield::p_Update(){
	DBG_ENTER("iPlayfield.Update")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1576>");
	p_UpdateGui();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1577>");
	p_OnUpdate();
}
void c_iPlayfield::p_AttachLast(){
	DBG_ENTER("iPlayfield.AttachLast")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<244>");
	bb_globals_iEnginePointer->m__playfieldList->p_AddLast2(this);
}
void c_iPlayfield::p_AttachLast2(c_iEngine* t_engine){
	DBG_ENTER("iPlayfield.AttachLast")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_engine,"engine")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<251>");
	t_engine->m__playfieldList->p_AddLast2(this);
}
void c_iPlayfield::p_AutoCls(int t_red,int t_green,int t_blue){
	DBG_ENTER("iPlayfield.AutoCls")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_red,"red")
	DBG_LOCAL(t_green,"green")
	DBG_LOCAL(t_blue,"blue")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<259>");
	m__clsRed=Float(t_red);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<260>");
	m__clsGreen=Float(t_green);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<261>");
	m__clsBlue=Float(t_blue);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<263>");
	m__autoCls=true;
}
void c_iPlayfield::p_AutoCls2(bool t_state){
	DBG_ENTER("iPlayfield.AutoCls")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_state,"state")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<271>");
	m__autoCls=t_state;
}
Float c_iPlayfield::p_Width(){
	DBG_ENTER("iPlayfield.Width")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1798>");
	Float t_=Float(m__width);
	return t_;
}
void c_iPlayfield::p_Width2(Float t_width){
	DBG_ENTER("iPlayfield.Width")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1805>");
	m__width=int(t_width);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1806>");
	m__vwidth=int(t_width);
}
Float c_iPlayfield::p_Height(){
	DBG_ENTER("iPlayfield.Height")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<521>");
	Float t_=Float(m__height);
	return t_;
}
void c_iPlayfield::p_Height2(Float t_height){
	DBG_ENTER("iPlayfield.Height")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<528>");
	m__height=int(t_height);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<529>");
	m__vheight=int(t_height);
}
void c_iPlayfield::p_Position(Float t_x,Float t_y){
	DBG_ENTER("iPlayfield.Position")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<764>");
	m__x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<765>");
	m__y=t_y;
}
void c_iPlayfield::p_Position2(Float t_x,Float t_y,Float t_z){
	DBG_ENTER("iPlayfield.Position")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_z,"z")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<774>");
	m__x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<775>");
	m__y=t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<776>");
	m__z=t_z;
}
Float c_iPlayfield::p_AlphaFade(){
	DBG_ENTER("iPlayfield.AlphaFade")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<216>");
	return m__alphaFade;
}
void c_iPlayfield::p_AlphaFade2(Float t_alpha){
	DBG_ENTER("iPlayfield.AlphaFade")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<223>");
	m__alphaFade=t_alpha;
}
Float c_iPlayfield::p_ZoomX(){
	DBG_ENTER("iPlayfield.ZoomX")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1903>");
	return m__zoomX;
}
void c_iPlayfield::p_ZoomX2(Float t_zoomX){
	DBG_ENTER("iPlayfield.ZoomX")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_zoomX,"zoomX")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1917>");
	m__zoomX=t_zoomX;
}
Float c_iPlayfield::p_ZoomY(){
	DBG_ENTER("iPlayfield.ZoomY")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1910>");
	return m__zoomY;
}
void c_iPlayfield::p_ZoomY2(Float t_zoomY){
	DBG_ENTER("iPlayfield.ZoomY")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_zoomY,"zoomY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1924>");
	m__zoomY=t_zoomY;
}
void c_iPlayfield::p_Remove(){
	DBG_ENTER("iPlayfield.Remove")
	c_iPlayfield *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<837>");
	if(m__tattoo){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<839>");
		m__enabled=0;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<843>");
		m__enabled=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<845>");
		bb_globals_iEnginePointer->m__playfieldList->p_Remove2(this);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<847>");
		if(m__autoDestroy){
			DBG_BLOCK();
			c_iEngineObject::m_destroyList->p_AddLast5(this);
		}
	}
}
void c_iPlayfield::mark(){
	c_iEngineObject::mark();
	gc_mark_q(m__layerList);
	gc_mark_q(m__currentLayer);
	gc_mark_q(m__guiList);
	gc_mark_q(m__guiLastObject);
}
String c_iPlayfield::debug(){
	String t="(iPlayfield)\n";
	t=c_iEngineObject::debug()+t;
	t+=dbg_decl("_alphaFade",&m__alphaFade);
	t+=dbg_decl("_autoCls",&m__autoCls);
	t+=dbg_decl("_backgroundRender",&m__backgroundRender);
	t+=dbg_decl("_cameraX",&m__cameraX);
	t+=dbg_decl("_cameraY",&m__cameraY);
	t+=dbg_decl("_clsBlue",&m__clsBlue);
	t+=dbg_decl("_clsGreen",&m__clsGreen);
	t+=dbg_decl("_clsRed",&m__clsRed);
	t+=dbg_decl("_colorFade",&m__colorFade);
	t+=dbg_decl("_currentLayer",&m__currentLayer);
	t+=dbg_decl("_guiEnabled",&m__guiEnabled);
	t+=dbg_decl("_guiLastObject",&m__guiLastObject);
	t+=dbg_decl("_guiList",&m__guiList);
	t+=dbg_decl("_guiPage",&m__guiPage);
	t+=dbg_decl("_layerList",&m__layerList);
	t+=dbg_decl("_nextGuiPage",&m__nextGuiPage);
	t+=dbg_decl("_rotation",&m__rotation);
	t+=dbg_decl("_scaleX",&m__scaleX);
	t+=dbg_decl("_scaleY",&m__scaleY);
	t+=dbg_decl("_visible",&m__visible);
	t+=dbg_decl("_vheight",&m__vheight);
	t+=dbg_decl("_vwidth",&m__vwidth);
	t+=dbg_decl("_x",&m__x);
	t+=dbg_decl("_y",&m__y);
	t+=dbg_decl("_z",&m__z);
	t+=dbg_decl("_zoomPointX",&m__zoomPointX);
	t+=dbg_decl("_zoomPointY",&m__zoomPointY);
	t+=dbg_decl("_zoomX",&m__zoomX);
	t+=dbg_decl("_zoomY",&m__zoomY);
	t+=dbg_decl("_width",&m__width);
	t+=dbg_decl("_height",&m__height);
	return t;
}
c_iList::c_iList(){
	m__length=0;
	m__index=0;
	m__data=Array<c_iPlayfield* >();
}
c_iList* c_iList::m_new(){
	DBG_ENTER("iList.new")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>");
	return this;
}
int c_iList::p_Length(){
	DBG_ENTER("iList.Length")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<170>");
	return m__length;
}
c_iPlayfield* c_iList::p_First(){
	DBG_ENTER("iList.First")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>");
	m__index=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>");
	if((m__length)!=0){
		DBG_BLOCK();
		return m__data.At(0);
	}
	return 0;
}
c_iPlayfield* c_iList::p_Ascend(){
	DBG_ENTER("iList.Ascend")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>");
	m__index=m__index+1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>");
	if(m__index>=0 && m__index<m__length){
		DBG_BLOCK();
		return m__data.At(m__index);
	}
	return 0;
}
c_iPlayfield* c_iList::p_Get(int t_index){
	DBG_ENTER("iList.Get")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<128>");
	return m__data.At(t_index);
}
void c_iList::p_AddLast2(c_iPlayfield* t_value){
	DBG_ENTER("iList.AddLast")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<55>");
	if(m__length==m__data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<56>");
		gc_assign(m__data,m__data.Resize(m__length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<58>");
	gc_assign(m__data.At(m__length),t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<59>");
	m__length=m__length+1;
}
int c_iList::p_Position3(c_iPlayfield* t_value){
	DBG_ENTER("iList.Position")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>");
	for(int t_i=0;t_i<m__length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>");
		if(m__data.At(t_i)==t_value){
			DBG_BLOCK();
			return t_i;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>");
	return -1;
}
void c_iList::p_RemoveFromIndex(int t_index){
	DBG_ENTER("iList.RemoveFromIndex")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>");
	for(int t_i=t_index;t_i<m__length-1;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>");
		gc_assign(m__data.At(t_i),m__data.At(t_i+1));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>");
	m__length=m__length-1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>");
	m__index=m__index-1;
}
void c_iList::p_Remove2(c_iPlayfield* t_value){
	DBG_ENTER("iList.Remove")
	c_iList *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>");
	int t_p=p_Position3(t_value);
	DBG_LOCAL(t_p,"p")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>");
	if(t_p>=0){
		DBG_BLOCK();
		p_RemoveFromIndex(t_p);
	}
}
void c_iList::mark(){
	Object::mark();
	gc_mark_q(m__data);
}
String c_iList::debug(){
	String t="(iList)\n";
	t+=dbg_decl("_data",&m__data);
	t+=dbg_decl("_index",&m__index);
	t+=dbg_decl("_length",&m__length);
	return t;
}
c_iStack2D::c_iStack2D(){
	m__alpha=FLOAT(.0);
	m__blend=0;
	m__color=Array<Float >();
	m__matrix=Array<Float >();
	m__scissor=Array<Float >();
}
int c_iStack2D::m__length;
Array<c_iStack2D* > c_iStack2D::m__data;
c_iStack2D* c_iStack2D::m_new(){
	DBG_ENTER("iStack2D.new")
	c_iStack2D *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<10>");
	return this;
}
c_iStack2D* c_iStack2D::m_Push(){
	DBG_ENTER("iStack2D.Push")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<48>");
	if(m__length==m__data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<50>");
		gc_assign(m__data,m__data.Resize(m__length*2+10));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<52>");
		for(int t_i=0;t_i<m__data.Length();t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<53>");
			if(!((m__data.At(t_i))!=0)){
				DBG_BLOCK();
				gc_assign(m__data.At(t_i),(new c_iStack2D)->m_new());
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<58>");
	c_iStack2D* t_o=m__data.At(m__length);
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<60>");
	t_o->m__alpha=bb_graphics_GetAlpha();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<61>");
	t_o->m__blend=bb_graphics_GetBlend();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<62>");
	gc_assign(t_o->m__color,bb_graphics_GetColor());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<63>");
	gc_assign(t_o->m__matrix,bb_graphics_GetMatrix());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<64>");
	gc_assign(t_o->m__scissor,bb_graphics_GetScissor());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<66>");
	m__length=m__length+1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<68>");
	return t_o;
}
void c_iStack2D::m_Pop(){
	DBG_ENTER("iStack2D.Pop")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<34>");
	m__length=m__length-1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<36>");
	c_iStack2D* t_o=m__data.At(m__length);
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<38>");
	bb_graphics_SetAlpha(t_o->m__alpha);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<39>");
	bb_graphics_SetBlend(t_o->m__blend);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<40>");
	bb_graphics_SetMatrix(t_o->m__matrix.At(0),t_o->m__matrix.At(1),t_o->m__matrix.At(2),t_o->m__matrix.At(3),t_o->m__matrix.At(4),t_o->m__matrix.At(5));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<41>");
	bb_graphics_SetColor(t_o->m__color.At(0),t_o->m__color.At(1),t_o->m__color.At(2));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<42>");
	bb_graphics_SetScissor(t_o->m__scissor.At(0),t_o->m__scissor.At(1),t_o->m__scissor.At(2),t_o->m__scissor.At(3));
}
void c_iStack2D::mark(){
	Object::mark();
	gc_mark_q(m__color);
	gc_mark_q(m__matrix);
	gc_mark_q(m__scissor);
}
String c_iStack2D::debug(){
	String t="(iStack2D)\n";
	t+=dbg_decl("_length",&c_iStack2D::m__length);
	t+=dbg_decl("_data",&c_iStack2D::m__data);
	t+=dbg_decl("_alpha",&m__alpha);
	t+=dbg_decl("_blend",&m__blend);
	t+=dbg_decl("_color",&m__color);
	t+=dbg_decl("_matrix",&m__matrix);
	t+=dbg_decl("_scissor",&m__scissor);
	return t;
}
Float bb_graphics_GetAlpha(){
	DBG_ENTER("GetAlpha")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<276>");
	return bb_graphics_context->m_alpha;
}
int bb_graphics_GetBlend(){
	DBG_ENTER("GetBlend")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<285>");
	return bb_graphics_context->m_blend;
}
Array<Float > bb_graphics_GetColor(){
	DBG_ENTER("GetColor")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<261>");
	Float t_2[]={bb_graphics_context->m_color_r,bb_graphics_context->m_color_g,bb_graphics_context->m_color_b};
	Array<Float > t_=Array<Float >(t_2,3);
	return t_;
}
int bb_graphics_GetColor2(Array<Float > t_color){
	DBG_ENTER("GetColor")
	DBG_LOCAL(t_color,"color")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<265>");
	t_color.At(0)=bb_graphics_context->m_color_r;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<266>");
	t_color.At(1)=bb_graphics_context->m_color_g;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<267>");
	t_color.At(2)=bb_graphics_context->m_color_b;
	return 0;
}
Array<Float > bb_graphics_GetMatrix(){
	DBG_ENTER("GetMatrix")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<323>");
	Float t_2[]={bb_graphics_context->m_ix,bb_graphics_context->m_iy,bb_graphics_context->m_jx,bb_graphics_context->m_jy,bb_graphics_context->m_tx,bb_graphics_context->m_ty};
	Array<Float > t_=Array<Float >(t_2,6);
	return t_;
}
int bb_graphics_GetMatrix2(Array<Float > t_matrix){
	DBG_ENTER("GetMatrix")
	DBG_LOCAL(t_matrix,"matrix")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<327>");
	t_matrix.At(0)=bb_graphics_context->m_ix;
	t_matrix.At(1)=bb_graphics_context->m_iy;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<328>");
	t_matrix.At(2)=bb_graphics_context->m_jx;
	t_matrix.At(3)=bb_graphics_context->m_jy;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<329>");
	t_matrix.At(4)=bb_graphics_context->m_tx;
	t_matrix.At(5)=bb_graphics_context->m_ty;
	return 0;
}
void bb_gfx_iSetScissor(Float t_x,Float t_y,Float t_width,Float t_height){
	DBG_ENTER("iSetScissor")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<504>");
	if(t_x<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<506>");
		if(-t_x>=t_width){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<507>");
			t_x=FLOAT(0.0);
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<508>");
			t_width=FLOAT(0.0);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<510>");
			t_width=t_width+t_x;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<511>");
			t_x=FLOAT(0.0);
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<516>");
	if(t_x+t_width>Float(bb_app_DeviceWidth())){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<518>");
		if(t_x>=Float(bb_app_DeviceWidth())){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<519>");
			t_x=Float(bb_app_DeviceWidth());
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<520>");
			t_width=FLOAT(0.0);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<522>");
			t_width=Float(bb_app_DeviceWidth()-int(t_x));
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<527>");
	if(t_y<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<529>");
		if(-t_y>=t_height){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<530>");
			t_y=FLOAT(0.0);
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<531>");
			t_height=FLOAT(0.0);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<533>");
			t_height=t_height+t_y;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<534>");
			t_y=FLOAT(0.0);
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<539>");
	if(t_y+t_height>Float(bb_app_DeviceHeight())){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<541>");
		if(t_y>=Float(bb_app_DeviceHeight())){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<542>");
			t_y=Float(bb_app_DeviceHeight());
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<543>");
			t_height=FLOAT(0.0);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<545>");
			t_height=Float(bb_app_DeviceHeight()-int(t_y));
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<550>");
	bb_graphics_SetScissor(t_x,t_y,t_width,t_height);
}
int bb_graphics_Transform(Float t_ix,Float t_iy,Float t_jx,Float t_jy,Float t_tx,Float t_ty){
	DBG_ENTER("Transform")
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_LOCAL(t_jx,"jx")
	DBG_LOCAL(t_jy,"jy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<355>");
	Float t_ix2=t_ix*bb_graphics_context->m_ix+t_iy*bb_graphics_context->m_jx;
	DBG_LOCAL(t_ix2,"ix2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<356>");
	Float t_iy2=t_ix*bb_graphics_context->m_iy+t_iy*bb_graphics_context->m_jy;
	DBG_LOCAL(t_iy2,"iy2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<357>");
	Float t_jx2=t_jx*bb_graphics_context->m_ix+t_jy*bb_graphics_context->m_jx;
	DBG_LOCAL(t_jx2,"jx2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<358>");
	Float t_jy2=t_jx*bb_graphics_context->m_iy+t_jy*bb_graphics_context->m_jy;
	DBG_LOCAL(t_jy2,"jy2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<359>");
	Float t_tx2=t_tx*bb_graphics_context->m_ix+t_ty*bb_graphics_context->m_jx+bb_graphics_context->m_tx;
	DBG_LOCAL(t_tx2,"tx2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<360>");
	Float t_ty2=t_tx*bb_graphics_context->m_iy+t_ty*bb_graphics_context->m_jy+bb_graphics_context->m_ty;
	DBG_LOCAL(t_ty2,"ty2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<361>");
	bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
	return 0;
}
int bb_graphics_Transform2(Array<Float > t_m){
	DBG_ENTER("Transform")
	DBG_LOCAL(t_m,"m")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<351>");
	bb_graphics_Transform(t_m.At(0),t_m.At(1),t_m.At(2),t_m.At(3),t_m.At(4),t_m.At(5));
	return 0;
}
int bb_graphics_Scale(Float t_x,Float t_y){
	DBG_ENTER("Scale")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<369>");
	bb_graphics_Transform(t_x,FLOAT(0.0),FLOAT(0.0),t_y,FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int bb_graphics_Translate(Float t_x,Float t_y){
	DBG_ENTER("Translate")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<365>");
	bb_graphics_Transform(FLOAT(1.0),FLOAT(0.0),FLOAT(0.0),FLOAT(1.0),t_x,t_y);
	return 0;
}
int bb_globals_iRotation;
int bb_graphics_Rotate(Float t_angle){
	DBG_ENTER("Rotate")
	DBG_LOCAL(t_angle,"angle")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<373>");
	bb_graphics_Transform((Float)cos((t_angle)*D2R),-(Float)sin((t_angle)*D2R),(Float)sin((t_angle)*D2R),(Float)cos((t_angle)*D2R),FLOAT(0.0),FLOAT(0.0));
	return 0;
}
c_iLayer::c_iLayer(){
	m__visible=true;
	m__stack2D=0;
	m__cameraSpeedX=FLOAT(1.0);
	m__cameraSpeedY=FLOAT(1.0);
	m__objectList=Array<Array<c_iList3* > >();
	m__blockWidth=0;
	m__blockWidthExtra=0;
	m__blockHeight=0;
	m__blockHeightExtra=0;
	m__lastUpdate=0;
	m__loaderCache=0;
	m__alphaFade=FLOAT(1.0);
	m__colorFade=FLOAT(1.0);
}
Float c_iLayer::p_CameraSpeedX(){
	DBG_ENTER("iLayer.CameraSpeedX")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<164>");
	return m__cameraSpeedX;
}
void c_iLayer::p_CameraSpeedX2(Float t_speedX){
	DBG_ENTER("iLayer.CameraSpeedX")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_speedX,"speedX")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<171>");
	m__cameraSpeedX=t_speedX;
}
Float c_iLayer::p_CameraSpeedY(){
	DBG_ENTER("iLayer.CameraSpeedY")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<178>");
	return m__cameraSpeedY;
}
void c_iLayer::p_CameraSpeedY2(Float t_speedY){
	DBG_ENTER("iLayer.CameraSpeedY")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_speedY,"speedY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<185>");
	m__cameraSpeedY=t_speedY;
}
int c_iLayer::p_OnRender(){
	DBG_ENTER("iLayer.OnRender")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<623>");
	return 0;
}
bool c_iLayer::p_Grid(){
	DBG_ENTER("iLayer.Grid")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<325>");
	if(m__objectList.Length()>1 || m__objectList.At(0).Length()>1){
		DBG_BLOCK();
		return true;
	}else{
		DBG_BLOCK();
		return false;
	}
}
int c_iLayer::p_GridWidth(){
	DBG_ENTER("iLayer.GridWidth")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<360>");
	int t_=m__objectList.Length();
	return t_;
}
int c_iLayer::p_GridHeight(){
	DBG_ENTER("iLayer.GridHeight")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<332>");
	int t_=m__objectList.At(0).Length();
	return t_;
}
void c_iLayer::p_RenderLayerObjects(){
	DBG_ENTER("iLayer.RenderLayerObjects")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<760>");
	if(p_Grid()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<762>");
		int t_startX=int(bb_globals_iEnginePointer->m__currentPlayfield->m__cameraX/Float(m__blockWidth)-Float(m__blockWidthExtra));
		DBG_LOCAL(t_startX,"startX")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<763>");
		int t_startY=int(bb_globals_iEnginePointer->m__currentPlayfield->m__cameraY/Float(m__blockHeight)-Float(m__blockHeightExtra));
		DBG_LOCAL(t_startY,"startY")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<765>");
		if(t_startX<0){
			DBG_BLOCK();
			t_startX=0;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<766>");
		if(t_startY<0){
			DBG_BLOCK();
			t_startY=0;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<768>");
		int t_stopX=int(Float(t_startX)+bb_globals_iEnginePointer->m__currentPlayfield->p_VWidth()/Float(m__blockWidth)+FLOAT(1.0)+FLOAT(1.0)+Float(m__blockWidthExtra*2));
		DBG_LOCAL(t_stopX,"stopX")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<769>");
		int t_stopY=int(Float(t_startY)+bb_globals_iEnginePointer->m__currentPlayfield->p_VHeight()/Float(m__blockHeight)+FLOAT(1.0)+FLOAT(1.0)+Float(m__blockHeightExtra*2));
		DBG_LOCAL(t_stopY,"stopY")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<771>");
		if(t_stopX>p_GridWidth()){
			DBG_BLOCK();
			t_stopX=p_GridWidth();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<772>");
		if(t_stopY>p_GridHeight()){
			DBG_BLOCK();
			t_stopY=p_GridHeight();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<774>");
		for(int t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y,"y")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<776>");
			for(int t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				DBG_BLOCK();
				DBG_LOCAL(t_x,"x")
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<778>");
				for(int t_i=0;t_i<m__objectList.At(t_x).At(t_y)->p_Length();t_i=t_i+1){
					DBG_BLOCK();
					DBG_LOCAL(t_i,"i")
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<779>");
					if(m__objectList.At(t_x).At(t_y)->p_Get(t_i)->m__enabled==1 && m__objectList.At(t_x).At(t_y)->p_Get(t_i)->m__visible){
						DBG_BLOCK();
						m__objectList.At(t_x).At(t_y)->p_Get(t_i)->p_Render();
					}
				}
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<788>");
		c_iLayerObject* t_o=m__objectList.At(0).At(0)->p_First();
		DBG_LOCAL(t_o,"o")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<789>");
		while((t_o)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<791>");
			if(t_o->m__enabled==1 && t_o->m__visible){
				DBG_BLOCK();
				t_o->p_Render();
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<793>");
			t_o=m__objectList.At(0).At(0)->p_Ascend();
		}
	}
}
int c_iLayer::p_OnTopRender(){
	DBG_ENTER("iLayer.OnTopRender")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<630>");
	return 0;
}
void c_iLayer::p_Render(){
	DBG_ENTER("iLayer.Render")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<723>");
	gc_assign(m__stack2D,c_iStack2D::m_Push());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<725>");
	if(p_CameraSpeedX()!=FLOAT(0.0) || p_CameraSpeedY()!=FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<726>");
		bb_graphics_Translate(bb_globals_iEnginePointer->m__currentPlayfield->p_PositionX()/bb_globals_iEnginePointer->m__currentPlayfield->m__scaleX-bb_globals_iEnginePointer->m__currentPlayfield->p_CameraX()*bb_globals_iEnginePointer->m__currentPlayfield->m__currentLayer->p_CameraSpeedX(),bb_globals_iEnginePointer->m__currentPlayfield->p_PositionY()/bb_globals_iEnginePointer->m__currentPlayfield->m__scaleY-bb_globals_iEnginePointer->m__currentPlayfield->p_CameraY()*bb_globals_iEnginePointer->m__currentPlayfield->m__currentLayer->p_CameraSpeedY());
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<729>");
	c_iStack2D::m_Push();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<730>");
	p_OnRender();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<731>");
	c_iStack2D::m_Pop();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<733>");
	p_RenderLayerObjects();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<735>");
	c_iStack2D::m_Push();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<736>");
	p_OnTopRender();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<737>");
	c_iStack2D::m_Pop();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<739>");
	c_iStack2D::m_Pop();
}
void c_iLayer::p_UpdateWorldXY(){
	DBG_ENTER("iLayer.UpdateWorldXY")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1000>");
	if(p_Grid()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1002>");
		int t_startX=int(bb_globals_iEnginePointer->m__currentPlayfield->m__cameraX/Float(m__blockWidth)-Float(m__blockWidthExtra));
		DBG_LOCAL(t_startX,"startX")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1003>");
		int t_startY=int(bb_globals_iEnginePointer->m__currentPlayfield->m__cameraY/Float(m__blockHeight)-Float(m__blockHeightExtra));
		DBG_LOCAL(t_startY,"startY")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1005>");
		if(t_startX<0){
			DBG_BLOCK();
			t_startX=0;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1006>");
		if(t_startY<0){
			DBG_BLOCK();
			t_startY=0;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1008>");
		int t_stopX=int(Float(t_startX)+bb_globals_iEnginePointer->m__currentPlayfield->p_VWidth()/Float(m__blockWidth)+FLOAT(1.0)+FLOAT(1.0)+Float(m__blockWidthExtra*2));
		DBG_LOCAL(t_stopX,"stopX")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1009>");
		int t_stopY=int(Float(t_startY)+bb_globals_iEnginePointer->m__currentPlayfield->p_VHeight()/Float(m__blockHeight)+FLOAT(1.0)+FLOAT(1.0)-Float(m__blockHeightExtra*2));
		DBG_LOCAL(t_stopY,"stopY")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1011>");
		if(t_stopX>p_GridWidth()){
			DBG_BLOCK();
			t_stopX=p_GridWidth();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1012>");
		if(t_stopY>p_GridHeight()){
			DBG_BLOCK();
			t_stopY=p_GridHeight();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1014>");
		for(int t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y,"y")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1016>");
			for(int t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				DBG_BLOCK();
				DBG_LOCAL(t_x,"x")
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1018>");
				for(int t_i=0;t_i<m__objectList.At(t_x).At(t_y)->p_Length();t_i=t_i+1){
					DBG_BLOCK();
					DBG_LOCAL(t_i,"i")
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1020>");
					m__objectList.At(t_x).At(t_y)->p_Get(t_i)->p_UpdateWorldXY();
				}
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1030>");
		c_iLayerObject* t_o=m__objectList.At(0).At(0)->p_First();
		DBG_LOCAL(t_o,"o")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1031>");
		while((t_o)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1033>");
			t_o->p_UpdateWorldXY();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1035>");
			t_o=m__objectList.At(0).At(0)->p_Ascend();
		}
	}
}
void c_iLayer::p_UpdateLayerObjects(){
	DBG_ENTER("iLayer.UpdateLayerObjects")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<927>");
	if(p_Grid()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<929>");
		int t_startX=int(bb_globals_iEnginePointer->m__currentPlayfield->m__cameraX/Float(m__blockWidth)-Float(m__blockWidthExtra));
		DBG_LOCAL(t_startX,"startX")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<930>");
		int t_startY=int(bb_globals_iEnginePointer->m__currentPlayfield->m__cameraY/Float(m__blockHeight)-Float(m__blockHeightExtra));
		DBG_LOCAL(t_startY,"startY")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<932>");
		if(t_startX<0){
			DBG_BLOCK();
			t_startX=0;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<933>");
		if(t_startY<0){
			DBG_BLOCK();
			t_startY=0;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<935>");
		int t_stopX=int(Float(t_startX)+bb_globals_iEnginePointer->m__currentPlayfield->p_VWidth()/Float(m__blockWidth)+FLOAT(1.0)+FLOAT(1.0)+Float(m__blockWidthExtra*2));
		DBG_LOCAL(t_stopX,"stopX")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<936>");
		int t_stopY=int(Float(t_startY)+bb_globals_iEnginePointer->m__currentPlayfield->p_VHeight()/Float(m__blockHeight)+FLOAT(1.0)+FLOAT(1.0)-Float(m__blockHeightExtra*2));
		DBG_LOCAL(t_stopY,"stopY")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<938>");
		if(t_stopX>p_GridWidth()){
			DBG_BLOCK();
			t_stopX=p_GridWidth();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<939>");
		if(t_stopY>p_GridHeight()){
			DBG_BLOCK();
			t_stopY=p_GridHeight();
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<941>");
		for(int t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y,"y")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<943>");
			for(int t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				DBG_BLOCK();
				DBG_LOCAL(t_x,"x")
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<945>");
				c_iLayerObject* t_o=m__objectList.At(t_x).At(t_y)->p_First();
				DBG_LOCAL(t_o,"o")
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<946>");
				while((t_o)!=0){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<948>");
					if(t_o->m__enabled==1){
						DBG_BLOCK();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<950>");
						t_o->p_Update();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<951>");
						t_o->p_OnUpdate();
					}else{
						DBG_BLOCK();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<955>");
						t_o->p_OnDisabled();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<957>");
						if(t_o->m__enabled>1){
							DBG_BLOCK();
							t_o->m__enabled=t_o->m__enabled-1;
						}
					}
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<961>");
					t_o=m__objectList.At(t_x).At(t_y)->p_Ascend();
				}
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<971>");
		c_iLayerObject* t_o2=m__objectList.At(0).At(0)->p_First();
		DBG_LOCAL(t_o2,"o")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<972>");
		while((t_o2)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<974>");
			if(t_o2->m__enabled==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<976>");
				t_o2->p_Update();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<977>");
				t_o2->p_OnUpdate();
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<981>");
				t_o2->p_OnDisabled();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<983>");
				if(t_o2->m__enabled>1){
					DBG_BLOCK();
					t_o2->m__enabled=t_o2->m__enabled-1;
				}
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<987>");
			t_o2=m__objectList.At(0).At(0)->p_Ascend();
		}
	}
}
int c_iLayer::p_OnUpdate(){
	DBG_ENTER("iLayer.OnUpdate")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<637>");
	return 0;
}
void c_iLayer::p_Update(){
	DBG_ENTER("iLayer.Update")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<914>");
	p_UpdateWorldXY();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<916>");
	p_UpdateLayerObjects();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<918>");
	p_OnUpdate();
}
void c_iLayer::p_Set2(){
	DBG_ENTER("iLayer.Set")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<831>");
	if((bb_globals_iEnginePointer)!=0){
		DBG_BLOCK();
		gc_assign(bb_globals_iEnginePointer->m__layerPointer,this);
	}
}
void c_iLayer::p_InitGrid(int t_width,int t_height){
	DBG_ENTER("iLayer.InitGrid")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<440>");
	gc_assign(m__objectList,m__objectList.Resize(t_width));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<442>");
	for(int t_x=0;t_x<t_width;t_x=t_x+1){
		DBG_BLOCK();
		DBG_LOCAL(t_x,"x")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<444>");
		if(m__objectList.At(t_x).Length()<t_height){
			DBG_BLOCK();
			gc_assign(m__objectList.At(t_x),m__objectList.At(t_x).Resize(t_height));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<446>");
		for(int t_y=0;t_y<t_height;t_y=t_y+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y,"y")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<447>");
			if(!((m__objectList.At(t_x).At(t_y))!=0)){
				DBG_BLOCK();
				gc_assign(m__objectList.At(t_x).At(t_y),(new c_iList3)->m_new());
			}
		}
	}
}
int c_iLayer::p_OnCreate(){
	DBG_ENTER("iLayer.OnCreate")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<616>");
	return 0;
}
void c_iLayer::p_SystemInit(){
	DBG_ENTER("iLayer.SystemInit")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<871>");
	p_Set2();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<873>");
	p_InitGrid(1,1);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<875>");
	p_OnCreate();
}
c_iLayer* c_iLayer::m_new(){
	DBG_ENTER("iLayer.new")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<598>");
	c_iEngineObject::m_new();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<599>");
	p_SystemInit();
	return this;
}
void c_iLayer::p_AttachLast(){
	DBG_ENTER("iLayer.AttachLast")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<142>");
	bb_globals_iEnginePointer->m__playfieldPointer->m__layerList->p_AddLast3(this);
}
void c_iLayer::p_AttachLast3(c_iPlayfield* t_playfield){
	DBG_ENTER("iLayer.AttachLast")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_playfield,"playfield")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<149>");
	t_playfield->m__layerList->p_AddLast3(this);
}
void c_iLayer::p_Destroy(){
	DBG_ENTER("iLayer.Destroy")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<256>");
	m__loaderCache=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<258>");
	m__objectList.At(0).At(0)=0;
}
void c_iLayer::p_Remove(){
	DBG_ENTER("iLayer.Remove")
	c_iLayer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<671>");
	if(m__tattoo){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<673>");
		m__enabled=0;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<677>");
		m__enabled=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<679>");
		for(int t_i=0;t_i<bb_globals_iEnginePointer->m__playfieldList->p_Length();t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<680>");
			bb_globals_iEnginePointer->m__playfieldList->p_Get(t_i)->m__layerList->p_Remove3(this);
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<683>");
		if(m__autoDestroy){
			DBG_BLOCK();
			c_iEngineObject::m_destroyList->p_AddLast5(this);
		}
	}
}
void c_iLayer::mark(){
	c_iEngineObject::mark();
	gc_mark_q(m__stack2D);
	gc_mark_q(m__objectList);
	gc_mark_q(m__loaderCache);
}
String c_iLayer::debug(){
	String t="(iLayer)\n";
	t=c_iEngineObject::debug()+t;
	t+=dbg_decl("_alphaFade",&m__alphaFade);
	t+=dbg_decl("_blockHeight",&m__blockHeight);
	t+=dbg_decl("_blockHeightExtra",&m__blockHeightExtra);
	t+=dbg_decl("_blockWidth",&m__blockWidth);
	t+=dbg_decl("_blockWidthExtra",&m__blockWidthExtra);
	t+=dbg_decl("_colorFade",&m__colorFade);
	t+=dbg_decl("_cameraSpeedX",&m__cameraSpeedX);
	t+=dbg_decl("_cameraSpeedY",&m__cameraSpeedY);
	t+=dbg_decl("_lastUpdate",&m__lastUpdate);
	t+=dbg_decl("_loaderCache",&m__loaderCache);
	t+=dbg_decl("_objectList",&m__objectList);
	t+=dbg_decl("_stack2D",&m__stack2D);
	t+=dbg_decl("_visible",&m__visible);
	return t;
}
c_iList2::c_iList2(){
	m__index=0;
	m__length=0;
	m__data=Array<c_iLayer* >();
}
c_iList2* c_iList2::m_new(){
	DBG_ENTER("iList.new")
	c_iList2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>");
	return this;
}
c_iLayer* c_iList2::p_First(){
	DBG_ENTER("iList.First")
	c_iList2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>");
	m__index=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>");
	if((m__length)!=0){
		DBG_BLOCK();
		return m__data.At(0);
	}
	return 0;
}
c_iLayer* c_iList2::p_Ascend(){
	DBG_ENTER("iList.Ascend")
	c_iList2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>");
	m__index=m__index+1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>");
	if(m__index>=0 && m__index<m__length){
		DBG_BLOCK();
		return m__data.At(m__index);
	}
	return 0;
}
void c_iList2::p_AddLast3(c_iLayer* t_value){
	DBG_ENTER("iList.AddLast")
	c_iList2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<55>");
	if(m__length==m__data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<56>");
		gc_assign(m__data,m__data.Resize(m__length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<58>");
	gc_assign(m__data.At(m__length),t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<59>");
	m__length=m__length+1;
}
int c_iList2::p_Position4(c_iLayer* t_value){
	DBG_ENTER("iList.Position")
	c_iList2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>");
	for(int t_i=0;t_i<m__length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>");
		if(m__data.At(t_i)==t_value){
			DBG_BLOCK();
			return t_i;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>");
	return -1;
}
void c_iList2::p_RemoveFromIndex(int t_index){
	DBG_ENTER("iList.RemoveFromIndex")
	c_iList2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>");
	for(int t_i=t_index;t_i<m__length-1;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>");
		gc_assign(m__data.At(t_i),m__data.At(t_i+1));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>");
	m__length=m__length-1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>");
	m__index=m__index-1;
}
void c_iList2::p_Remove3(c_iLayer* t_value){
	DBG_ENTER("iList.Remove")
	c_iList2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>");
	int t_p=p_Position4(t_value);
	DBG_LOCAL(t_p,"p")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>");
	if(t_p>=0){
		DBG_BLOCK();
		p_RemoveFromIndex(t_p);
	}
}
void c_iList2::mark(){
	Object::mark();
	gc_mark_q(m__data);
}
String c_iList2::debug(){
	String t="(iList)\n";
	t+=dbg_decl("_data",&m__data);
	t+=dbg_decl("_index",&m__index);
	t+=dbg_decl("_length",&m__length);
	return t;
}
c_iGraph::c_iGraph(){
	m__visible=true;
	m__ghost=false;
	m__height=0;
	m__scaleY=FLOAT(1.0);
	m__scaleX=FLOAT(1.0);
	m__rotation=FLOAT(.0);
	m__width=0;
	m__x=FLOAT(.0);
	m__y=FLOAT(.0);
	m__z=FLOAT(.0);
	m__mcPosition=(new c_iVector2d)->m_new(FLOAT(0.0),FLOAT(0.0));
	m__mcVelocity=(new c_iVector2d)->m_new(FLOAT(0.0),FLOAT(0.0));
	m__ghostBlend=0;
	m__ghostAlpha=FLOAT(1.0);
	m__ghostRed=FLOAT(255.0);
	m__ghostGreen=FLOAT(255.0);
	m__ghostBlue=FLOAT(255.0);
	m__blend=0;
	m__alpha=FLOAT(1.0);
	m__red=FLOAT(255.0);
	m__green=FLOAT(255.0);
	m__blue=FLOAT(255.0);
}
Float c_iGraph::p_Height(){
	DBG_ENTER("iGraph.Height")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<480>");
	Float t_=Float(m__height)*m__scaleY;
	return t_;
}
void c_iGraph::p_Height2(Float t_height){
	DBG_ENTER("iGraph.Height")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<487>");
	m__height=int(t_height);
}
Float c_iGraph::p_ScaleX(){
	DBG_ENTER("iGraph.ScaleX")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<628>");
	return m__scaleX;
}
void c_iGraph::p_ScaleX2(Float t_scaleX){
	DBG_ENTER("iGraph.ScaleX")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_scaleX,"scaleX")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<635>");
	m__scaleX=t_scaleX;
}
Float c_iGraph::p_Rotation(){
	DBG_ENTER("iGraph.Rotation")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<594>");
	return m__rotation;
}
void c_iGraph::p_Rotation2(Float t_angle){
	DBG_ENTER("iGraph.Rotation")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_angle,"angle")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<601>");
	m__rotation=t_angle;
}
Float c_iGraph::p_Width(){
	DBG_ENTER("iGraph.Width")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<670>");
	Float t_=Float(m__width)*m__scaleX;
	return t_;
}
void c_iGraph::p_Width2(Float t_width){
	DBG_ENTER("iGraph.Width")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<677>");
	m__width=int(t_width);
}
Float c_iGraph::p_PositionX(){
	DBG_ENTER("iGraph.PositionX")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<494>");
	return m__x;
}
void c_iGraph::p_PositionX2(Float t_x){
	DBG_ENTER("iGraph.PositionX")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<501>");
	m__x=t_x;
}
void c_iGraph::p_PositionX3(Float t_x,c_iGraph* t_graph){
	DBG_ENTER("iGraph.PositionX")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_graph,"graph")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<508>");
	m__x=t_graph->m__x+t_x;
}
Float c_iGraph::p_PositionY(){
	DBG_ENTER("iGraph.PositionY")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<515>");
	return m__y;
}
void c_iGraph::p_PositionY2(Float t_y){
	DBG_ENTER("iGraph.PositionY")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<522>");
	m__y=t_y;
}
void c_iGraph::p_PositionY3(Float t_y,c_iGraph* t_graph){
	DBG_ENTER("iGraph.PositionY")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_graph,"graph")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<529>");
	m__y=t_graph->m__y+t_y;
}
c_iGraph* c_iGraph::m_new(){
	DBG_ENTER("iGraph.new")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<10>");
	c_iEngineObject::m_new();
	return this;
}
Float c_iGraph::p_PositionZ(){
	DBG_ENTER("iGraph.PositionZ")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<536>");
	return m__z;
}
void c_iGraph::p_PositionZ2(Float t_z){
	DBG_ENTER("iGraph.PositionZ")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_z,"z")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<543>");
	m__z=t_z;
}
void c_iGraph::p_PositionZ3(Float t_z,c_iGraph* t_graph){
	DBG_ENTER("iGraph.PositionZ")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_z,"z")
	DBG_LOCAL(t_graph,"graph")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<550>");
	m__z=t_graph->m__z+t_z;
}
void c_iGraph::p_Destroy(){
	DBG_ENTER("iGraph.Destroy")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<347>");
	m__mcPosition=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<348>");
	m__mcVelocity=0;
}
Float c_iGraph::p_ScaleY(){
	DBG_ENTER("iGraph.ScaleY")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<642>");
	return m__scaleY;
}
void c_iGraph::p_ScaleY2(Float t_scaleY){
	DBG_ENTER("iGraph.ScaleY")
	c_iGraph *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_scaleY,"scaleY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<649>");
	m__scaleY=t_scaleY;
}
void c_iGraph::mark(){
	c_iEngineObject::mark();
	gc_mark_q(m__mcPosition);
	gc_mark_q(m__mcVelocity);
}
String c_iGraph::debug(){
	String t="(iGraph)\n";
	t=c_iEngineObject::debug()+t;
	t+=dbg_decl("_mcPosition",&m__mcPosition);
	t+=dbg_decl("_mcVelocity",&m__mcVelocity);
	t+=dbg_decl("_alpha",&m__alpha);
	t+=dbg_decl("_blend",&m__blend);
	t+=dbg_decl("_blue",&m__blue);
	t+=dbg_decl("_ghost",&m__ghost);
	t+=dbg_decl("_ghostAlpha",&m__ghostAlpha);
	t+=dbg_decl("_ghostBlend",&m__ghostBlend);
	t+=dbg_decl("_ghostBlue",&m__ghostBlue);
	t+=dbg_decl("_ghostGreen",&m__ghostGreen);
	t+=dbg_decl("_ghostRed",&m__ghostRed);
	t+=dbg_decl("_green",&m__green);
	t+=dbg_decl("_height",&m__height);
	t+=dbg_decl("_red",&m__red);
	t+=dbg_decl("_rotation",&m__rotation);
	t+=dbg_decl("_scaleX",&m__scaleX);
	t+=dbg_decl("_scaleY",&m__scaleY);
	t+=dbg_decl("_visible",&m__visible);
	t+=dbg_decl("_width",&m__width);
	t+=dbg_decl("_x",&m__x);
	t+=dbg_decl("_y",&m__y);
	t+=dbg_decl("_z",&m__z);
	return t;
}
c_iLayerObject::c_iLayerObject(){
	m__worldHeight=FLOAT(.0);
	m__worldScaleX=FLOAT(.0);
	m__worldScaleY=FLOAT(.0);
	m__worldRotation=FLOAT(.0);
	m__worldWidth=FLOAT(.0);
	m__worldX=FLOAT(.0);
	m__worldY=FLOAT(.0);
	m__parent=0;
	m__layer=c_iLayerObject::m_AutoLayer();
	m__scoreCollector=0;
	m__cosine=FLOAT(.0);
	m__cosineRadius=FLOAT(.0);
	m__sine=FLOAT(.0);
	m__sineRadius=FLOAT(.0);
	m__stamina=1;
	m__invincible=false;
	m__hits=0;
	m__scoreCount=false;
	m__scoreBoard=0;
	m__points=0;
	m__countDown=false;
	m__collisionRead=false;
	m__collisionWrite=false;
	m__column=0;
	m__row=0;
	m__loaderCache=0;
	m__collisionMask=Array<int >();
	m__control=Array<c_iControlSet* >();
	m__debugInfo=Array<String >(8);
}
void c_iLayerObject::p_Render(){
	DBG_ENTER("iLayerObject.Render")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
}
void c_iLayerObject::p_UpdateWorldXY(){
	DBG_ENTER("iLayerObject.UpdateWorldXY")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2604>");
	m__worldHeight=bb_math2_Abs2(p_Height());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2605>");
	m__worldScaleX=p_ScaleX();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2606>");
	m__worldScaleY=p_ScaleX();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2607>");
	m__worldRotation=p_Rotation();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2608>");
	m__worldWidth=bb_math2_Abs2(p_Width());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2609>");
	m__worldX=m__x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2610>");
	m__worldY=m__y;
}
void c_iLayerObject::p_Update(){
	DBG_ENTER("iLayerObject.Update")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
}
int c_iLayerObject::p_OnUpdate(){
	DBG_ENTER("iLayerObject.OnUpdate")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1693>");
	return 0;
}
int c_iLayerObject::p_OnDisabled(){
	DBG_ENTER("iLayerObject.OnDisabled")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1613>");
	return 0;
}
c_iLayer* c_iLayerObject::m_AutoLayer(){
	DBG_ENTER("iLayerObject.AutoLayer")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2749>");
	if(!((bb_globals_iEnginePointer)!=0)){
		DBG_BLOCK();
		(new c_iEngine)->m_new();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2750>");
	if(!((bb_globals_iEnginePointer->m__playfieldPointer)!=0)){
		DBG_BLOCK();
		(new c_iPlayfield)->m_new();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2751>");
	if(!((bb_globals_iEnginePointer->m__layerPointer)!=0)){
		DBG_BLOCK();
		(new c_iLayer)->m_new();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2753>");
	return bb_globals_iEnginePointer->m__layerPointer;
}
c_iLayer* c_iLayerObject::p_Layer(){
	DBG_ENTER("iLayerObject.Layer")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1126>");
	if((m__parent)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1127>");
		return m__parent->m__layer;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1129>");
		return m__layer;
	}
}
void c_iLayerObject::p_Layer2(c_iLayer* t_layer){
	DBG_ENTER("iLayerObject.Layer")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layer,"layer")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1138>");
	gc_assign(m__layer,t_layer);
}
Float c_iLayerObject::p_Cosine(){
	DBG_ENTER("iLayerObject.Cosine")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<832>");
	return m__cosine;
}
void c_iLayerObject::p_Cosine2(Float t_cosine){
	DBG_ENTER("iLayerObject.Cosine")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_cosine,"cosine")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<839>");
	m__cosine=t_cosine;
}
Float c_iLayerObject::p_CosineRadius(){
	DBG_ENTER("iLayerObject.CosineRadius")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<846>");
	return m__cosineRadius;
}
void c_iLayerObject::p_CosineRadius2(Float t_cosineRadius){
	DBG_ENTER("iLayerObject.CosineRadius")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_cosineRadius,"cosineRadius")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<853>");
	m__cosineRadius=t_cosineRadius;
}
Float c_iLayerObject::p_RenderX(){
	DBG_ENTER("iLayerObject.RenderX")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1950>");
	Float t_=p_PositionX()+(Float)cos((p_Cosine())*D2R)*p_CosineRadius();
	return t_;
}
Float c_iLayerObject::p_Sine(){
	DBG_ENTER("iLayerObject.Sine")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2085>");
	return m__sine;
}
void c_iLayerObject::p_Sine2(Float t_sine){
	DBG_ENTER("iLayerObject.Sine")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_sine,"sine")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2092>");
	m__sine=t_sine;
}
Float c_iLayerObject::p_SineRadius(){
	DBG_ENTER("iLayerObject.SineRadius")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2099>");
	return m__sineRadius;
}
void c_iLayerObject::p_SineRadius2(Float t_sineRadius){
	DBG_ENTER("iLayerObject.SineRadius")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_sineRadius,"sineRadius")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2106>");
	m__sineRadius=t_sineRadius;
}
Float c_iLayerObject::p_RenderY(){
	DBG_ENTER("iLayerObject.RenderY")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1957>");
	Float t_=p_PositionY()+(Float)sin((p_Sine())*D2R)*p_SineRadius();
	return t_;
}
int c_iLayerObject::p_CollisionMethod(c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerObject.CollisionMethod")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<657>");
	if(c_iCollision::m_Rectangles(p_RenderX(),p_RenderY(),int(p_Width()),int(p_Height()),t_layerObject->p_RenderX(),t_layerObject->p_RenderY(),int(t_layerObject->p_Width()),int(t_layerObject->p_Height()),1)){
		DBG_BLOCK();
		return 1;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<658>");
	return 0;
}
int c_iLayerObject::p_Collides(c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerObject.Collides")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<639>");
	if(m__enabled==1 && t_layerObject->m__enabled==1 && t_layerObject!=this && t_layerObject!=m__scoreCollector){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<640>");
		int t_=p_CollisionMethod(t_layerObject);
		return t_;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<643>");
	return 0;
}
int c_iLayerObject::p_OnOutro(){
	DBG_ENTER("iLayerObject.OnOutro")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1622>");
	return 0;
}
int c_iLayerObject::p_OnRemove(){
	DBG_ENTER("iLayerObject.OnRemove")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1670>");
	return 0;
}
void c_iLayerObject::p_Remove(){
	DBG_ENTER("iLayerObject.Remove")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1809>");
	if((m__layer)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1811>");
		if(m__enabled==1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1813>");
			if(m__countDown){
				DBG_BLOCK();
				bb_globals_iEnginePointer->m__countDown=bb_globals_iEnginePointer->m__countDown-1;
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1815>");
			m__enabled=0;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1819>");
		if(m__tattoo==false){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1821>");
			if(m__collisionRead){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1822>");
				bb_globals_iEnginePointer->m__collisionReadList->p_Remove4(this);
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1823>");
				m__collisionRead=false;
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1826>");
			if(m__collisionWrite){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1827>");
				bb_globals_iEnginePointer->m__collisionWriteList->p_Remove4(this);
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1828>");
				m__collisionWrite=false;
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1831>");
			m__layer->m__objectList.At(m__column).At(m__row)->p_Remove4(this);
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1833>");
			p_OnRemove();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1835>");
			if(m__autoDestroy){
				DBG_BLOCK();
				c_iEngineObject::m_destroyList->p_AddLast5(this);
			}
		}
	}
}
void c_iLayerObject::p_ScoreSystem(c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerObject.ScoreSystem")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2136>");
	int t_s1=m__stamina;
	DBG_LOCAL(t_s1,"s1")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2137>");
	int t_s2=t_layerObject->m__stamina;
	DBG_LOCAL(t_s2,"s2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2139>");
	if(m__invincible){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2140>");
		if(t_layerObject->m__enabled==1){
			DBG_BLOCK();
			t_layerObject->m__stamina=0;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2142>");
		if(t_layerObject->m__enabled==1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2143>");
			m__stamina=m__stamina-t_s2;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2144>");
			m__hits=m__hits+1;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2145>");
			m__ghost=true;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2149>");
	if(t_layerObject->m__invincible){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2150>");
		if(m__enabled==1){
			DBG_BLOCK();
			m__stamina=0;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2152>");
		if(m__enabled==1){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2153>");
			t_layerObject->m__stamina=t_layerObject->m__stamina-t_s1;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2154>");
			t_layerObject->m__hits=t_layerObject->m__hits+1;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2155>");
			t_layerObject->m__ghost=true;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2159>");
	if(m__stamina<1){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2161>");
		m__stamina=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2163>");
		if(m__scoreCount){
			DBG_BLOCK();
			bb_globals_iEnginePointer->m__scoreCount=bb_globals_iEnginePointer->m__scoreCount+1;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2165>");
		if((t_layerObject->m__scoreCollector)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2166>");
			t_layerObject->m__scoreCollector->m__scoreBoard=t_layerObject->m__scoreCollector->m__scoreBoard+m__points;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2168>");
			t_layerObject->m__scoreBoard=t_layerObject->m__scoreBoard+m__points;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2171>");
		p_OnOutro();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2172>");
		p_Remove();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2176>");
	if(t_layerObject->m__stamina<1){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2178>");
		t_layerObject->m__stamina=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2180>");
		if(t_layerObject->m__scoreCount){
			DBG_BLOCK();
			bb_globals_iEnginePointer->m__scoreCount=bb_globals_iEnginePointer->m__scoreCount+1;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2182>");
		if((m__scoreCollector)!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2183>");
			m__scoreCollector->m__scoreBoard=m__scoreCollector->m__scoreBoard+t_layerObject->m__points;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2185>");
			m__scoreBoard=m__scoreBoard+t_layerObject->m__points;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2188>");
		t_layerObject->p_OnOutro();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2189>");
		t_layerObject->p_Remove();
	}
}
int c_iLayerObject::p_OnCollision(c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerObject.OnCollision")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1605>");
	p_ScoreSystem(t_layerObject);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1606>");
	return 0;
}
c_iLayerObject* c_iLayerObject::m_new(){
	DBG_ENTER("iLayerObject.new")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<13>");
	c_iGraph::m_new();
	return this;
}
int c_iLayerObject::p_OnAttach(){
	DBG_ENTER("iLayerObject.OnAttach")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1596>");
	return 0;
}
void c_iLayerObject::p_Attach(){
	DBG_ENTER("iLayerObject.Attach")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<507>");
	p_OnAttach();
}
void c_iLayerObject::p_AttachLast(){
	DBG_ENTER("iLayerObject.AttachLast")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<461>");
	if((m__layer)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<462>");
		m__layer->m__objectList.At(m__column).At(m__row)->p_Remove4(this);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<463>");
		m__column=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<464>");
		m__row=0;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<467>");
	bb_globals_iEnginePointer->m__layerPointer->m__objectList.At(m__column).At(m__row)->p_AddLast4(this);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<468>");
	gc_assign(m__layer,bb_globals_iEnginePointer->m__layerPointer);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<470>");
	p_Attach();
}
void c_iLayerObject::p_AttachLast4(c_iLayer* t_layer){
	DBG_ENTER("iLayerObject.AttachLast")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layer,"layer")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<479>");
	if((m__layer)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<480>");
		m__layer->m__objectList.At(m__column).At(m__row)->p_Remove4(this);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<481>");
		m__column=0;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<482>");
		m__row=0;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<485>");
	t_layer->m__objectList.At(m__column).At(m__row)->p_AddLast4(this);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<486>");
	gc_assign(m__layer,t_layer);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<488>");
	p_Attach();
}
c_iLayerObject* c_iLayerObject::p_Parent(){
	DBG_ENTER("iLayerObject.Parent")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1042>");
	return m__parent;
}
void c_iLayerObject::p_Parent2(c_iLayerObject* t_parent){
	DBG_ENTER("iLayerObject.Parent")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_parent,"parent")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1049>");
	gc_assign(m__parent,t_parent);
}
void c_iLayerObject::p_AttachLast5(c_iLayerGroup* t_layerGroup){
	DBG_ENTER("iLayerObject.AttachLast")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layerGroup,"layerGroup")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<497>");
	t_layerGroup->p_AddItemLast(this);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<499>");
	p_Attach();
}
void c_iLayerObject::p_Position(Float t_x,Float t_y){
	DBG_ENTER("iLayerObject.Position")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1743>");
	m__x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1744>");
	m__y=t_y;
}
void c_iLayerObject::p_Position5(Float t_x,Float t_y,c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerObject.Position")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1753>");
	m__x=t_layerObject->m__x+t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1754>");
	m__y=t_layerObject->m__y+t_y;
}
void c_iLayerObject::p_Position2(Float t_x,Float t_y,Float t_z){
	DBG_ENTER("iLayerObject.Position")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_z,"z")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1763>");
	m__x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1764>");
	m__y=t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1765>");
	m__z=t_z;
}
void c_iLayerObject::p_Position6(Float t_x,Float t_y,Float t_z,c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerObject.Position")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_z,"z")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1774>");
	m__x=t_layerObject->p_PositionX()+t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1775>");
	m__y=t_layerObject->p_PositionY()+t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1776>");
	m__z=t_layerObject->p_PositionZ()+t_z;
}
void c_iLayerObject::p_Destroy(){
	DBG_ENTER("iLayerObject.Destroy")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<861>");
	c_iGraph::p_Destroy();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<863>");
	m__loaderCache=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<865>");
	m__collisionMask=Array<int >();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<866>");
	m__control=Array<c_iControlSet* >();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<867>");
	m__debugInfo=Array<String >();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<868>");
	m__layer=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<869>");
	m__parent=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<870>");
	m__scoreCollector=0;
}
int c_iLayerObject::p_SetAlpha(Float t_alpha){
	DBG_ENTER("iLayerObject.SetAlpha")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2049>");
	bb_graphics_SetAlpha(t_alpha*bb_globals_iEnginePointer->m__alphaFade*bb_globals_iEnginePointer->m__currentPlayfield->m__alphaFade*m__layer->m__alphaFade);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2051>");
	return 0;
}
int c_iLayerObject::p_SetColor(Float t_red,Float t_green,Float t_blue){
	DBG_ENTER("iLayerObject.SetColor")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_red,"red")
	DBG_LOCAL(t_green,"green")
	DBG_LOCAL(t_blue,"blue")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2063>");
	bb_graphics_SetColor(t_red*bb_globals_iEnginePointer->m__colorFade*bb_globals_iEnginePointer->m__currentPlayfield->m__colorFade*m__layer->m__colorFade,t_green*bb_globals_iEnginePointer->m__colorFade*bb_globals_iEnginePointer->m__currentPlayfield->m__colorFade*m__layer->m__colorFade,t_blue*bb_globals_iEnginePointer->m__colorFade*bb_globals_iEnginePointer->m__currentPlayfield->m__colorFade*m__layer->m__colorFade);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2065>");
	return 0;
}
void c_iLayerObject::p_RenderDebugInfo(){
	DBG_ENTER("iLayerObject.RenderDebugInfo")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1883>");
	if(((m__debugInfo).Length()!=0) && ((m__debugInfo.At(0)).Length()!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1885>");
		bb_graphics_SetBlend(0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1886>");
		p_SetAlpha(FLOAT(1.0));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1887>");
		p_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1889>");
		for(int t_i=0;t_i<m__debugInfo.Length();t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1890>");
			if((m__debugInfo.At(t_i)).Length()!=0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1891>");
				bb_graphics_DrawText(m__debugInfo.At(t_i),p_RenderX()-p_Width()*FLOAT(.5),p_RenderY()+p_Height()*FLOAT(.5)+Float(t_i*12),FLOAT(0.0),FLOAT(0.0));
			}
		}
	}
}
void c_iLayerObject::p_ClearDebugInfo(){
	DBG_ENTER("iLayerObject.ClearDebugInfo")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<544>");
	if(((m__debugInfo).Length()!=0) && ((m__debugInfo.At(0)).Length()!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<545>");
		for(int t_i=0;t_i<m__debugInfo.Length();t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<546>");
			m__debugInfo.At(t_i)=String();
		}
	}
}
void c_iLayerObject::p_RenderObject(c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerObject.RenderObject")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1921>");
	gc_assign(t_layerObject->m__layer,m__layer);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1923>");
	if(t_layerObject->m__enabled==1 && t_layerObject->m__visible){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1925>");
		bb_graphics_PushMatrix();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1928>");
		bb_graphics_Translate(p_RenderX(),p_RenderY());
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1929>");
		bb_graphics_Rotate(p_Rotation()*Float(bb_globals_iRotation));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1930>");
		bb_graphics_Scale(p_ScaleX(),p_ScaleY());
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1932>");
		Float t_alpha=t_layerObject->m__alpha;
		DBG_LOCAL(t_alpha,"alpha")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1934>");
		t_layerObject->m__alpha=t_layerObject->m__alpha*m__alpha;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1936>");
		t_layerObject->p_Render();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1938>");
		t_layerObject->m__alpha=m__alpha;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1940>");
		bb_graphics_PopMatrix();
	}
}
void c_iLayerObject::p_RenderList(c_iList3* t_list){
	DBG_ENTER("iLayerObject.RenderList")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_list,"list")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1904>");
	c_iLayerObject* t_o=t_list->p_First();
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1906>");
	while((t_o)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1908>");
		p_RenderObject(t_o);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1910>");
		t_o=t_list->p_Ascend();
	}
}
bool c_iLayerObject::p_TestForRender(){
	DBG_ENTER("iLayerObject.TestForRender")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2330>");
	if(m__scaleX==FLOAT(0.0)){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2331>");
	if(m__scaleY==FLOAT(0.0)){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2332>");
	if(m__alpha==FLOAT(0.0)){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2334>");
	return true;
}
void c_iLayerObject::p_UpdateObject(c_iLayerObject* t_LayerObject){
	DBG_ENTER("iLayerObject.UpdateObject")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_LayerObject,"LayerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2524>");
	gc_assign(t_LayerObject->m__layer,m__layer);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2526>");
	if(p_TestForRender()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2528>");
		t_LayerObject->m__x=t_LayerObject->m__x*p_ScaleX();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2529>");
		t_LayerObject->m__y=t_LayerObject->m__y*p_ScaleY();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2531>");
		t_LayerObject->m__x=t_LayerObject->m__x+p_RenderX();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2532>");
		t_LayerObject->m__y=t_LayerObject->m__y+p_RenderY();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2534>");
		t_LayerObject->m__rotation=t_LayerObject->m__rotation+m__rotation;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2536>");
		t_LayerObject->m__scaleX=t_LayerObject->m__scaleX*m__scaleX;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2537>");
		t_LayerObject->m__scaleY=t_LayerObject->m__scaleY*m__scaleY;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2538>");
		t_LayerObject->m__alpha=t_LayerObject->m__alpha*m__alpha;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2540>");
		if(m__rotation==FLOAT(0.0)){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2542>");
			if(t_LayerObject->m__enabled==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2544>");
				t_LayerObject->p_Update();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2545>");
				t_LayerObject->p_OnUpdate();
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2549>");
				t_LayerObject->p_OnDisabled();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2551>");
				if(t_LayerObject->m__enabled>1){
					DBG_BLOCK();
					t_LayerObject->m__enabled=t_LayerObject->m__enabled-1;
				}
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2557>");
			Float t_a=(Float)(atan2(t_LayerObject->m__y-m__y,t_LayerObject->m__x-m__x)*R2D)+m__rotation*Float(-bb_globals_iRotation);
			DBG_LOCAL(t_a,"a")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2558>");
			Float t_d=(Float)sqrt((Float)pow(t_LayerObject->m__y-m__y,FLOAT(2.0))+(Float)pow(t_LayerObject->m__x-m__x,FLOAT(2.0)));
			DBG_LOCAL(t_d,"d")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2559>");
			Float t_tx=m__x+(Float)cos((t_a)*D2R)*t_d;
			DBG_LOCAL(t_tx,"tx")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2560>");
			Float t_ty=m__y+(Float)sin((t_a)*D2R)*t_d;
			DBG_LOCAL(t_ty,"ty")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2561>");
			t_tx=t_tx-t_LayerObject->m__x;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2562>");
			t_ty=t_ty-t_LayerObject->m__y;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2563>");
			t_LayerObject->m__x=t_LayerObject->m__x+t_tx;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2564>");
			t_LayerObject->m__y=t_LayerObject->m__y+t_ty;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2566>");
			if(t_LayerObject->m__enabled==1){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2568>");
				t_LayerObject->p_Update();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2569>");
				t_LayerObject->p_OnUpdate();
			}else{
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2573>");
				t_LayerObject->p_OnDisabled();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2575>");
				if(t_LayerObject->m__enabled>1){
					DBG_BLOCK();
					t_LayerObject->m__enabled=t_LayerObject->m__enabled-1;
				}
			}
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2579>");
			t_LayerObject->m__x=t_LayerObject->m__x-t_tx;
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2580>");
			t_LayerObject->m__y=t_LayerObject->m__y-t_ty;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2584>");
		t_LayerObject->m__x=t_LayerObject->m__x-p_RenderX();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2585>");
		t_LayerObject->m__y=t_LayerObject->m__y-p_RenderY();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2587>");
		t_LayerObject->m__x=t_LayerObject->m__x/p_ScaleX();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2588>");
		t_LayerObject->m__y=t_LayerObject->m__y/p_ScaleY();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2590>");
		t_LayerObject->m__rotation=t_LayerObject->m__rotation-m__rotation;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2592>");
		t_LayerObject->m__scaleX=t_LayerObject->m__scaleX/m__scaleX;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2593>");
		t_LayerObject->m__scaleY=t_LayerObject->m__scaleY/m__scaleY;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2594>");
		t_LayerObject->m__alpha=t_LayerObject->m__alpha/m__alpha;
	}
}
void c_iLayerObject::p_UpdateList(c_iList3* t_list){
	DBG_ENTER("iLayerObject.UpdateList")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_list,"list")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2508>");
	c_iLayerObject* t_o=t_list->p_First();
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2509>");
	while((t_o)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2511>");
		p_UpdateObject(t_o);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2513>");
		t_o=t_list->p_Ascend();
	}
}
void c_iLayerObject::p_UpdateWorldXYObject(c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerObject.UpdateWorldXYObject")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2634>");
	Float t_x=t_layerObject->m__x;
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2635>");
	Float t_y=t_layerObject->m__y;
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2636>");
	Float t_rotation=t_layerObject->m__rotation;
	DBG_LOCAL(t_rotation,"rotation")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2637>");
	Float t_scaleX=t_layerObject->m__scaleX;
	DBG_LOCAL(t_scaleX,"scaleX")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2638>");
	Float t_scaleY=t_layerObject->m__scaleY;
	DBG_LOCAL(t_scaleY,"scaleY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2640>");
	t_layerObject->m__x=t_layerObject->m__x*p_ScaleX();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2641>");
	t_layerObject->m__y=t_layerObject->m__y*p_ScaleY();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2642>");
	t_layerObject->m__x=t_layerObject->m__x+p_PositionX();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2643>");
	t_layerObject->m__y=t_layerObject->m__y+p_PositionY();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2644>");
	t_layerObject->m__rotation=t_layerObject->m__rotation+m__rotation;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2645>");
	t_layerObject->m__scaleX=t_layerObject->m__scaleX*m__scaleX;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2646>");
	t_layerObject->m__scaleY=t_layerObject->m__scaleY*m__scaleY;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2648>");
	Float t_a=(Float)(atan2(t_layerObject->m__y-m__y,t_layerObject->m__x-m__x)*R2D)+m__rotation*Float(-bb_globals_iRotation);
	DBG_LOCAL(t_a,"a")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2649>");
	Float t_d=(Float)sqrt((Float)pow(t_layerObject->m__y-m__y,FLOAT(2.0))+(Float)pow(t_layerObject->m__x-m__x,FLOAT(2.0)));
	DBG_LOCAL(t_d,"d")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2650>");
	Float t_tx=m__x+(Float)cos((t_a)*D2R)*t_d;
	DBG_LOCAL(t_tx,"tx")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2651>");
	Float t_ty=m__y+(Float)sin((t_a)*D2R)*t_d;
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2652>");
	t_tx=t_tx-t_layerObject->m__x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2653>");
	t_ty=t_ty-t_layerObject->m__y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2654>");
	t_layerObject->m__x=t_layerObject->m__x+t_tx;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2655>");
	t_layerObject->m__y=t_layerObject->m__y+t_ty;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2657>");
	t_layerObject->p_UpdateWorldXY();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2659>");
	t_layerObject->m__x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2660>");
	t_layerObject->m__y=t_y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2661>");
	t_layerObject->m__rotation=t_rotation;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2662>");
	t_layerObject->m__scaleX=t_scaleX;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2663>");
	t_layerObject->m__scaleY=t_scaleY;
}
void c_iLayerObject::p_UpdateWorldXYList(c_iList3* t_list){
	DBG_ENTER("iLayerObject.UpdateWorldXYList")
	c_iLayerObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_list,"list")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2618>");
	c_iLayerObject* t_o=t_list->p_First();
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2619>");
	while((t_o)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2621>");
		p_UpdateWorldXYObject(t_o);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2623>");
		t_o=t_list->p_Ascend();
	}
}
void c_iLayerObject::mark(){
	c_iGraph::mark();
	gc_mark_q(m__parent);
	gc_mark_q(m__layer);
	gc_mark_q(m__scoreCollector);
	gc_mark_q(m__loaderCache);
	gc_mark_q(m__collisionMask);
	gc_mark_q(m__control);
	gc_mark_q(m__debugInfo);
}
String c_iLayerObject::debug(){
	String t="(iLayerObject)\n";
	t=c_iGraph::debug()+t;
	t+=dbg_decl("_layer",&m__layer);
	t+=dbg_decl("_parent",&m__parent);
	t+=dbg_decl("_collisionMask",&m__collisionMask);
	t+=dbg_decl("_collisionRead",&m__collisionRead);
	t+=dbg_decl("_collisionWrite",&m__collisionWrite);
	t+=dbg_decl("_column",&m__column);
	t+=dbg_decl("_control",&m__control);
	t+=dbg_decl("_cosine",&m__cosine);
	t+=dbg_decl("_cosineRadius",&m__cosineRadius);
	t+=dbg_decl("_countDown",&m__countDown);
	t+=dbg_decl("_debugInfo",&m__debugInfo);
	t+=dbg_decl("_hits",&m__hits);
	t+=dbg_decl("_invincible",&m__invincible);
	t+=dbg_decl("_loaderCache",&m__loaderCache);
	t+=dbg_decl("_points",&m__points);
	t+=dbg_decl("_row",&m__row);
	t+=dbg_decl("_scoreBoard",&m__scoreBoard);
	t+=dbg_decl("_scoreCollector",&m__scoreCollector);
	t+=dbg_decl("_scoreCount",&m__scoreCount);
	t+=dbg_decl("_sine",&m__sine);
	t+=dbg_decl("_sineRadius",&m__sineRadius);
	t+=dbg_decl("_stamina",&m__stamina);
	t+=dbg_decl("_worldRotation",&m__worldRotation);
	t+=dbg_decl("_worldScaleX",&m__worldScaleX);
	t+=dbg_decl("_worldScaleY",&m__worldScaleY);
	t+=dbg_decl("_worldHeight",&m__worldHeight);
	t+=dbg_decl("_worldWidth",&m__worldWidth);
	t+=dbg_decl("_worldX",&m__worldX);
	t+=dbg_decl("_worldY",&m__worldY);
	return t;
}
c_iList3::c_iList3(){
	m__length=0;
	m__data=Array<c_iLayerObject* >();
	m__index=0;
}
int c_iList3::p_Length(){
	DBG_ENTER("iList.Length")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<170>");
	return m__length;
}
c_iLayerObject* c_iList3::p_Get(int t_index){
	DBG_ENTER("iList.Get")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<128>");
	return m__data.At(t_index);
}
c_iLayerObject* c_iList3::p_First(){
	DBG_ENTER("iList.First")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>");
	m__index=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>");
	if((m__length)!=0){
		DBG_BLOCK();
		return m__data.At(0);
	}
	return 0;
}
c_iLayerObject* c_iList3::p_Ascend(){
	DBG_ENTER("iList.Ascend")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>");
	m__index=m__index+1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>");
	if(m__index>=0 && m__index<m__length){
		DBG_BLOCK();
		return m__data.At(m__index);
	}
	return 0;
}
c_iList3* c_iList3::m_new(){
	DBG_ENTER("iList.new")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>");
	return this;
}
int c_iList3::p_Position7(c_iLayerObject* t_value){
	DBG_ENTER("iList.Position")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>");
	for(int t_i=0;t_i<m__length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>");
		if(m__data.At(t_i)==t_value){
			DBG_BLOCK();
			return t_i;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>");
	return -1;
}
void c_iList3::p_RemoveFromIndex(int t_index){
	DBG_ENTER("iList.RemoveFromIndex")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>");
	for(int t_i=t_index;t_i<m__length-1;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>");
		gc_assign(m__data.At(t_i),m__data.At(t_i+1));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>");
	m__length=m__length-1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>");
	m__index=m__index-1;
}
void c_iList3::p_Remove4(c_iLayerObject* t_value){
	DBG_ENTER("iList.Remove")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>");
	int t_p=p_Position7(t_value);
	DBG_LOCAL(t_p,"p")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>");
	if(t_p>=0){
		DBG_BLOCK();
		p_RemoveFromIndex(t_p);
	}
}
void c_iList3::p_AddLast4(c_iLayerObject* t_value){
	DBG_ENTER("iList.AddLast")
	c_iList3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<55>");
	if(m__length==m__data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<56>");
		gc_assign(m__data,m__data.Resize(m__length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<58>");
	gc_assign(m__data.At(m__length),t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<59>");
	m__length=m__length+1;
}
void c_iList3::mark(){
	Object::mark();
	gc_mark_q(m__data);
}
String c_iList3::debug(){
	String t="(iList)\n";
	t+=dbg_decl("_data",&m__data);
	t+=dbg_decl("_index",&m__index);
	t+=dbg_decl("_length",&m__length);
	return t;
}
c_iGuiObject::c_iGuiObject(){
	m__worldHeight=FLOAT(.0);
	m__worldScaleX=FLOAT(.0);
	m__worldScaleY=FLOAT(.0);
	m__worldRotation=FLOAT(.0);
	m__worldWidth=FLOAT(.0);
	m__worldX=FLOAT(.0);
	m__worldY=FLOAT(.0);
	m__multiTouch=false;
	m__touchIndex=0;
	m__parent=0;
	m__playfield=c_iGuiObject::m_AutoPlayfield();
	m__handleX=0;
	m__handleY=0;
	m__touchDown=0;
	m__over=false;
	m__rememberTopObject=true;
	m__debugInfo=Array<String >(8);
}
void c_iGuiObject::p_RenderGhost(){
	DBG_ENTER("iGuiObject.RenderGhost")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
}
void c_iGuiObject::p_Render(){
	DBG_ENTER("iGuiObject.Render")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
}
c_iGuiObject* c_iGuiObject::m__topObject;
void c_iGuiObject::p_UpdateWorldXY(){
	DBG_ENTER("iGuiObject.UpdateWorldXY")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2018>");
	m__worldHeight=bb_math2_Abs2(p_Height());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2019>");
	m__worldScaleX=p_ScaleX();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2020>");
	m__worldScaleY=p_ScaleX();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2021>");
	m__worldRotation=p_Rotation();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2022>");
	m__worldWidth=bb_math2_Abs2(p_Width());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2023>");
	m__worldX=m__x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2024>");
	m__worldY=m__y;
}
c_iGuiObject* c_iGuiObject::p_Parent(){
	DBG_ENTER("iGuiObject.Parent")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1193>");
	return m__parent;
}
void c_iGuiObject::p_Parent3(c_iGuiObject* t_parent){
	DBG_ENTER("iGuiObject.Parent")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_parent,"parent")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1200>");
	gc_assign(m__parent,t_parent);
}
c_iPlayfield* c_iGuiObject::m_AutoPlayfield(){
	DBG_ENTER("iGuiObject.AutoPlayfield")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2163>");
	if(!((bb_globals_iEnginePointer)!=0)){
		DBG_BLOCK();
		(new c_iEngine)->m_new();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2164>");
	if(!((bb_globals_iEnginePointer->m__playfieldPointer)!=0)){
		DBG_BLOCK();
		(new c_iPlayfield)->m_new();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2166>");
	return bb_globals_iEnginePointer->m__playfieldPointer;
}
void c_iGuiObject::p_Playfield(c_iPlayfield* t_playfield){
	DBG_ENTER("iGuiObject.Playfield")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_playfield,"playfield")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1344>");
	gc_assign(m__playfield,t_playfield);
}
c_iPlayfield* c_iGuiObject::p_Playfield2(){
	DBG_ENTER("iGuiObject.Playfield")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1332>");
	if((p_Parent())!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1333>");
		c_iPlayfield* t_=p_Parent()->p_Playfield2();
		return t_;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1335>");
		return m__playfield;
	}
}
Float c_iGuiObject::p_TouchX(int t_index){
	DBG_ENTER("iGuiObject.TouchX")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1724>");
	if(!((bb_globals_iEnginePointer)!=0) || !((p_Playfield2())!=0)){
		DBG_BLOCK();
		Float t_=bb_input_TouchX(t_index);
		return t_;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1726>");
	Float t_2=bb_input_TouchX(t_index)/p_Playfield2()->m__scaleX-p_Playfield2()->m__x/p_Playfield2()->m__scaleX;
	return t_2;
}
Float c_iGuiObject::p_TouchY(int t_index){
	DBG_ENTER("iGuiObject.TouchY")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1735>");
	if(!((bb_globals_iEnginePointer)!=0) || !((p_Playfield2())!=0)){
		DBG_BLOCK();
		Float t_=bb_input_TouchY(t_index);
		return t_;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1737>");
	Float t_2=bb_input_TouchY(t_index)/p_Playfield2()->m__scaleY-p_Playfield2()->m__y/p_Playfield2()->m__scaleY;
	return t_2;
}
bool c_iGuiObject::p_TouchOver(int t_index){
	DBG_ENTER("iGuiObject.TouchOver")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1711>");
	if(p_Playfield2()!=(c_iSystemGui::m__playfield) && p_Playfield2()!=bb_functions_iGetTouchPlayfield(t_index)){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1713>");
	if(bb_commoncode_iPointInsideRectange(m__worldX,m__worldY,m__worldWidth,m__worldHeight,p_TouchX(t_index),p_TouchY(t_index),m__worldRotation*Float(bb_globals_iRotation),Float(m__handleX)*bb_math2_Abs2(m__worldScaleX),Float(m__handleY)*bb_math2_Abs2(m__worldScaleY))){
		DBG_BLOCK();
		return true;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1715>");
	return false;
}
bool c_iGuiObject::p_GetTouchIndex(){
	DBG_ENTER("iGuiObject.GetTouchIndex")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<853>");
	for(m__touchIndex=0;m__touchIndex<bb_functions_iTouchDowns();m__touchIndex=m__touchIndex+1){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<854>");
		if(((bb_input_TouchDown(m__touchIndex))!=0) && p_TouchOver(m__touchIndex)){
			DBG_BLOCK();
			return true;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<857>");
	return false;
}
void c_iGuiObject::p_UpdateInput(){
	DBG_ENTER("iGuiObject.UpdateInput")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1945>");
	if(m__multiTouch){
		DBG_BLOCK();
		p_GetTouchIndex();
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1947>");
	m__touchDown=bb_input_TouchDown(m__touchIndex);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1949>");
	m__over=p_TouchOver(m__touchIndex);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1951>");
	if(m__rememberTopObject){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1953>");
		if(!m__multiTouch && ((m__topObject)!=0)){
			DBG_BLOCK();
			m__over=false;
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1955>");
		if(!((m__topObject)!=0) && m__over){
			DBG_BLOCK();
			gc_assign(m__topObject,this);
		}
	}
}
void c_iGuiObject::p_UpdateGhost(){
	DBG_ENTER("iGuiObject.UpdateGhost")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
}
void c_iGuiObject::p_ClearDebugInfo(){
	DBG_ENTER("iGuiObject.ClearDebugInfo")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<749>");
	if(((m__debugInfo).Length()!=0) && ((m__debugInfo.At(0)).Length()!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<750>");
		for(int t_i=0;t_i<m__debugInfo.Length();t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<751>");
			m__debugInfo.At(t_i)=String();
		}
	}
}
void c_iGuiObject::p_Update(){
	DBG_ENTER("iGuiObject.Update")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1821>");
	p_ClearDebugInfo();
}
int c_iGuiObject::p_OnUpdate(){
	DBG_ENTER("iGuiObject.OnUpdate")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1170>");
	return 0;
}
void c_iGuiObject::p_Destroy(){
	DBG_ENTER("iGuiObject.Destroy")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<775>");
	c_iGraph::p_Destroy();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<777>");
	m__parent=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<778>");
	m__playfield=0;
}
int c_iGuiObject::p_OnRemove(){
	DBG_ENTER("iGuiObject.OnRemove")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1160>");
	return 0;
}
void c_iGuiObject::p_Remove(){
	DBG_ENTER("iGuiObject.Remove")
	c_iGuiObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1425>");
	if(p_Playfield2()->m__guiLastObject==this){
		DBG_BLOCK();
		p_Playfield2()->m__guiLastObject=0;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1427>");
	p_Disable();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1429>");
	if(m__tattoo==false){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1431>");
		p_OnRemove();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1433>");
		for(int t_i=0;t_i<p_Playfield2()->m__guiList.Length();t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1434>");
			p_Playfield2()->m__guiList.At(t_i)->p_Remove5(this);
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1437>");
		if(m__autoDestroy){
			DBG_BLOCK();
			c_iEngineObject::m_destroyList->p_AddLast5(this);
		}
	}
}
void c_iGuiObject::mark(){
	c_iGraph::mark();
	gc_mark_q(m__parent);
	gc_mark_q(m__playfield);
	gc_mark_q(m__debugInfo);
}
String c_iGuiObject::debug(){
	String t="(iGuiObject)\n";
	t=c_iGraph::debug()+t;
	t+=dbg_decl("_topObject",&c_iGuiObject::m__topObject);
	t+=dbg_decl("_parent",&m__parent);
	t+=dbg_decl("_playfield",&m__playfield);
	t+=dbg_decl("_debugInfo",&m__debugInfo);
	t+=dbg_decl("_handleX",&m__handleX);
	t+=dbg_decl("_handleY",&m__handleY);
	t+=dbg_decl("_multiTouch",&m__multiTouch);
	t+=dbg_decl("_over",&m__over);
	t+=dbg_decl("_rememberTopObject",&m__rememberTopObject);
	t+=dbg_decl("_touchDown",&m__touchDown);
	t+=dbg_decl("_touchIndex",&m__touchIndex);
	t+=dbg_decl("_worldRotation",&m__worldRotation);
	t+=dbg_decl("_worldScaleX",&m__worldScaleX);
	t+=dbg_decl("_worldScaleY",&m__worldScaleY);
	t+=dbg_decl("_worldHeight",&m__worldHeight);
	t+=dbg_decl("_worldWidth",&m__worldWidth);
	t+=dbg_decl("_worldX",&m__worldX);
	t+=dbg_decl("_worldY",&m__worldY);
	return t;
}
c_iList4::c_iList4(){
	m__index=0;
	m__length=0;
	m__data=Array<c_iGuiObject* >();
}
c_iGuiObject* c_iList4::p_First(){
	DBG_ENTER("iList.First")
	c_iList4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>");
	m__index=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>");
	if((m__length)!=0){
		DBG_BLOCK();
		return m__data.At(0);
	}
	return 0;
}
c_iGuiObject* c_iList4::p_Ascend(){
	DBG_ENTER("iList.Ascend")
	c_iList4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>");
	m__index=m__index+1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>");
	if(m__index>=0 && m__index<m__length){
		DBG_BLOCK();
		return m__data.At(m__index);
	}
	return 0;
}
c_iList4* c_iList4::m_new(){
	DBG_ENTER("iList.new")
	c_iList4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>");
	return this;
}
c_iGuiObject* c_iList4::p_Last(){
	DBG_ENTER("iList.Last")
	c_iList4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<160>");
	m__index=m__length-1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<162>");
	if((m__length)!=0){
		DBG_BLOCK();
		c_iGuiObject* t_=m__data.At(m__length-1);
		return t_;
	}else{
		DBG_BLOCK();
		return 0;
	}
}
c_iGuiObject* c_iList4::p_Descend(){
	DBG_ENTER("iList.Descend")
	c_iList4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<107>");
	m__index=m__index-1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<109>");
	if(m__index>=0 && m__index<m__length){
		DBG_BLOCK();
		return m__data.At(m__index);
	}
	return 0;
}
int c_iList4::p_Position8(c_iGuiObject* t_value){
	DBG_ENTER("iList.Position")
	c_iList4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>");
	for(int t_i=0;t_i<m__length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>");
		if(m__data.At(t_i)==t_value){
			DBG_BLOCK();
			return t_i;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>");
	return -1;
}
void c_iList4::p_RemoveFromIndex(int t_index){
	DBG_ENTER("iList.RemoveFromIndex")
	c_iList4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>");
	for(int t_i=t_index;t_i<m__length-1;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>");
		gc_assign(m__data.At(t_i),m__data.At(t_i+1));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>");
	m__length=m__length-1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>");
	m__index=m__index-1;
}
void c_iList4::p_Remove5(c_iGuiObject* t_value){
	DBG_ENTER("iList.Remove")
	c_iList4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>");
	int t_p=p_Position8(t_value);
	DBG_LOCAL(t_p,"p")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>");
	if(t_p>=0){
		DBG_BLOCK();
		p_RemoveFromIndex(t_p);
	}
}
void c_iList4::mark(){
	Object::mark();
	gc_mark_q(m__data);
}
String c_iList4::debug(){
	String t="(iList)\n";
	t+=dbg_decl("_data",&m__data);
	t+=dbg_decl("_index",&m__index);
	t+=dbg_decl("_length",&m__length);
	return t;
}
int bb_graphics_DrawRect(Float t_x,Float t_y,Float t_w,Float t_h){
	DBG_ENTER("DrawRect")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_w,"w")
	DBG_LOCAL(t_h,"h")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<393>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<395>");
	bb_graphics_context->p_Validate();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<396>");
	bb_graphics_renderDevice->DrawRect(t_x,t_y,t_w,t_h);
	return 0;
}
c_iSystemGui::c_iSystemGui(){
}
c_iSystemGui* c_iSystemGui::m__playfield;
c_iPlayfield* c_iSystemGui::m_Playfield(){
	DBG_ENTER("iSystemGui.Playfield")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<107>");
	c_iPlayfield* t_=(m__playfield);
	return t_;
}
void c_iSystemGui::m_GuiPage(int t_page){
	DBG_ENTER("iSystemGui.GuiPage")
	DBG_LOCAL(t_page,"page")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<114>");
	if((m_Playfield())!=0){
		DBG_BLOCK();
		m_Playfield()->p_GuiPage2(t_page);
	}
}
void c_iSystemGui::p_Set2(){
	DBG_ENTER("iSystemGui.Set")
	c_iSystemGui *self=this;
	DBG_LOCAL(self,"Self")
}
int c_iSystemGui::p_SetAlpha(Float t_alpha){
	DBG_ENTER("iSystemGui.SetAlpha")
	c_iSystemGui *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<233>");
	bb_graphics_SetAlpha(t_alpha);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<234>");
	return 0;
}
int c_iSystemGui::p_SetColor(Float t_red,Float t_green,Float t_blue){
	DBG_ENTER("iSystemGui.SetColor")
	c_iSystemGui *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_red,"red")
	DBG_LOCAL(t_green,"green")
	DBG_LOCAL(t_blue,"blue")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<241>");
	bb_graphics_SetColor(t_red,t_green,t_blue);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<242>");
	return 0;
}
void c_iSystemGui::mark(){
	c_iPlayfield::mark();
}
String c_iSystemGui::debug(){
	String t="(iSystemGui)\n";
	t=c_iPlayfield::debug()+t;
	t+=dbg_decl("_playfield",&c_iSystemGui::m__playfield);
	return t;
}
c_iTask::c_iTask(){
	m__visible=1;
	m__cold=true;
	m__started=false;
}
int c_iTask::p_OnRender(){
	DBG_ENTER("iTask.OnRender")
	c_iTask *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<80>");
	return 0;
}
void c_iTask::p_Render(){
	DBG_ENTER("iTask.Render")
	c_iTask *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<184>");
	p_OnRender();
}
void c_iTask::m_Render(c_iList5* t_list){
	DBG_ENTER("iTask.Render")
	DBG_LOCAL(t_list,"list")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<192>");
	if((t_list)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<194>");
		if((t_list->p_Length())!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<196>");
			c_iTask* t_t=t_list->p_First();
			DBG_LOCAL(t_t,"t")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<197>");
			while((t_t)!=0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<199>");
				if((t_t->m__enabled)!=0){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<201>");
					if(t_t->m__visible==1){
						DBG_BLOCK();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<202>");
						t_t->p_Render();
					}else{
						DBG_BLOCK();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<204>");
						if(t_t->m__visible>1){
							DBG_BLOCK();
							t_t->m__visible=t_t->m__visible-1;
						}
					}
				}
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<209>");
				t_t=t_list->p_Ascend();
			}
		}
	}
}
int c_iTask::p_OnUpdate(){
	DBG_ENTER("iTask.OnUpdate")
	c_iTask *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<87>");
	return 0;
}
void c_iTask::p_Update(){
	DBG_ENTER("iTask.Update")
	c_iTask *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<237>");
	p_OnUpdate();
}
void c_iTask::p_SystemStart(){
	DBG_ENTER("iTask.SystemStart")
	c_iTask *self=this;
	DBG_LOCAL(self,"Self")
}
int c_iTask::p_OnColdStart(){
	DBG_ENTER("iTask.OnColdStart")
	c_iTask *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<50>");
	return 0;
}
int c_iTask::p_OnStart(){
	DBG_ENTER("iTask.OnStart")
	c_iTask *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<66>");
	return 0;
}
void c_iTask::m_Update(c_iList5* t_list){
	DBG_ENTER("iTask.Update")
	DBG_LOCAL(t_list,"list")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<245>");
	if((t_list)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<247>");
		if((t_list->p_Length())!=0){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<249>");
			c_iTask* t_t=t_list->p_First();
			DBG_LOCAL(t_t,"t")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<251>");
			while((t_t)!=0){
				DBG_BLOCK();
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<253>");
				if(t_t->m__enabled==1){
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<255>");
					if(t_t->m__cold==true || t_t->m__started==false){
						DBG_BLOCK();
						t_t->p_SystemStart();
					}
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<257>");
					if(t_t->m__cold==true){
						DBG_BLOCK();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<258>");
						t_t->p_OnColdStart();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<259>");
						t_t->m__cold=false;
					}
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<262>");
					if(t_t->m__started==false){
						DBG_BLOCK();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<263>");
						t_t->p_OnStart();
						DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<264>");
						t_t->m__started=true;
					}
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<267>");
					t_t->p_Update();
				}else{
					DBG_BLOCK();
					DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<271>");
					if(t_t->m__enabled>1){
						DBG_BLOCK();
						t_t->m__enabled=t_t->m__enabled-1;
					}
				}
				DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<275>");
				t_t=t_list->p_Ascend();
			}
		}
	}
}
void c_iTask::mark(){
	c_iObject::mark();
}
String c_iTask::debug(){
	String t="(iTask)\n";
	t=c_iObject::debug()+t;
	t+=dbg_decl("_cold",&m__cold);
	t+=dbg_decl("_started",&m__started);
	t+=dbg_decl("_visible",&m__visible);
	return t;
}
c_iList5::c_iList5(){
	m__length=0;
	m__index=0;
	m__data=Array<c_iTask* >();
}
c_iList5* c_iList5::m_new(){
	DBG_ENTER("iList.new")
	c_iList5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>");
	return this;
}
int c_iList5::p_Length(){
	DBG_ENTER("iList.Length")
	c_iList5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<170>");
	return m__length;
}
c_iTask* c_iList5::p_First(){
	DBG_ENTER("iList.First")
	c_iList5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>");
	m__index=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>");
	if((m__length)!=0){
		DBG_BLOCK();
		return m__data.At(0);
	}
	return 0;
}
c_iTask* c_iList5::p_Ascend(){
	DBG_ENTER("iList.Ascend")
	c_iList5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>");
	m__index=m__index+1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>");
	if(m__index>=0 && m__index<m__length){
		DBG_BLOCK();
		return m__data.At(m__index);
	}
	return 0;
}
void c_iList5::mark(){
	Object::mark();
	gc_mark_q(m__data);
}
String c_iList5::debug(){
	String t="(iList)\n";
	t+=dbg_decl("_data",&m__data);
	t+=dbg_decl("_index",&m__index);
	t+=dbg_decl("_length",&m__length);
	return t;
}
c_iList5* bb_globals_iTaskList;
int bb_math2_Abs(int t_x){
	DBG_ENTER("Abs")
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<46>");
	if(t_x>=0){
		DBG_BLOCK();
		return t_x;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<47>");
	int t_=-t_x;
	return t_;
}
Float bb_math2_Abs2(Float t_x){
	DBG_ENTER("Abs")
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<73>");
	if(t_x>=FLOAT(0.0)){
		DBG_BLOCK();
		return t_x;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<74>");
	Float t_=-t_x;
	return t_;
}
int bb_globals_iMultiTouch;
int bb_input_TouchDown(int t_index){
	DBG_ENTER("TouchDown")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<84>");
	int t_=((bb_input_device->p_KeyDown(384+t_index))?1:0);
	return t_;
}
int bb_functions_iTouchDowns(){
	DBG_ENTER("iTouchDowns")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<442>");
	int t_i=0;
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<444>");
	for(t_i=0;t_i<bb_globals_iMultiTouch;t_i=t_i+1){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<445>");
		if((bb_input_TouchDown(t_i))!=0){
			DBG_BLOCK();
			t_i=t_i+1;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<448>");
	return t_i;
}
Float bb_input_TouchX(int t_index){
	DBG_ENTER("TouchX")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<76>");
	Float t_=bb_input_device->p_TouchX(t_index);
	return t_;
}
Float bb_input_TouchY(int t_index){
	DBG_ENTER("TouchY")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<80>");
	Float t_=bb_input_device->p_TouchY(t_index);
	return t_;
}
c_iPlayfield* bb_functions_iGetTouchPlayfield(int t_index){
	DBG_ENTER("iGetTouchPlayfield")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<163>");
	if(bb_globals_iEnginePointer->m__playfieldList->p_Length()==0 && ((bb_globals_iEnginePointer->m__playfieldPointer)!=0) && ((bb_globals_iEnginePointer->m__playfieldPointer->m__enabled)!=0)){
		DBG_BLOCK();
		return bb_globals_iEnginePointer->m__playfieldPointer;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<165>");
	if(bb_globals_iEnginePointer->m__playfieldList->p_Length()==1 && ((bb_globals_iEnginePointer->m__playfieldList->p_First()->m__enabled)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<167>");
		c_iPlayfield* t_p=bb_globals_iEnginePointer->m__playfieldList->p_First();
		DBG_LOCAL(t_p,"p")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<169>");
		if(bb_input_TouchX(t_index)>=t_p->m__x && bb_input_TouchY(t_index)>=t_p->m__y && bb_input_TouchX(t_index)<=t_p->m__x+Float(t_p->m__vwidth)*t_p->m__scaleX && bb_input_TouchY(t_index)<=t_p->m__y+Float(t_p->m__vheight)*t_p->m__scaleY){
			DBG_BLOCK();
			return t_p;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<173>");
		for(int t_i=bb_globals_iEnginePointer->m__playfieldList->p_Length()-1;t_i>=0;t_i=t_i+-1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<175>");
			c_iPlayfield* t_p2=bb_globals_iEnginePointer->m__playfieldList->p_Get(t_i);
			DBG_LOCAL(t_p2,"p")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<177>");
			if(bb_input_TouchX(t_index)>=t_p2->m__x && bb_input_TouchY(t_index)>=t_p2->m__y && bb_input_TouchX(t_index)<=t_p2->m__x+Float(t_p2->m__vwidth)*t_p2->m__scaleX && bb_input_TouchY(t_index)<=t_p2->m__y+Float(t_p2->m__vheight)*t_p2->m__scaleY){
				DBG_BLOCK();
				return t_p2;
			}
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<183>");
	return 0;
}
bool bb_commoncode_iPointInsideRectange(Float t_x,Float t_y,Float t_width,Float t_height,Float t_pointX,Float t_pointY,Float t_rotation,Float t_handleX,Float t_handleY){
	DBG_ENTER("iPointInsideRectange")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_LOCAL(t_pointX,"pointX")
	DBG_LOCAL(t_pointY,"pointY")
	DBG_LOCAL(t_rotation,"rotation")
	DBG_LOCAL(t_handleX,"handleX")
	DBG_LOCAL(t_handleY,"handleY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<92>");
	Float t_c=(Float)cos((t_rotation)*D2R);
	DBG_LOCAL(t_c,"c")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<93>");
	Float t_s=(Float)sin((t_rotation)*D2R);
	DBG_LOCAL(t_s,"s")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<95>");
	Float t_rotatedX=t_x+t_c*(t_pointX-t_x)-t_s*(t_pointY-t_y)+t_handleX;
	DBG_LOCAL(t_rotatedX,"rotatedX")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<96>");
	Float t_rotatedY=t_y+t_s*(t_pointX-t_x)+t_c*(t_pointY-t_y)+t_handleY;
	DBG_LOCAL(t_rotatedY,"rotatedY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<98>");
	Float t_leftX=t_x;
	DBG_LOCAL(t_leftX,"leftX")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<99>");
	Float t_rightX=t_x+t_width;
	DBG_LOCAL(t_rightX,"rightX")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<100>");
	Float t_topY=t_y;
	DBG_LOCAL(t_topY,"topY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<101>");
	Float t_bottomY=t_y+t_height;
	DBG_LOCAL(t_bottomY,"bottomY")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<103>");
	bool t_=t_leftX<=t_rotatedX && t_rotatedX<=t_rightX && t_topY<=t_rotatedY && t_rotatedY<=t_bottomY;
	return t_;
}
c_iCollision::c_iCollision(){
}
bool c_iCollision::m_Rectangles(Float t_x1,Float t_y1,int t_width1,int t_height1,Float t_x2,Float t_y2,int t_width2,int t_height2,int t_centerRect){
	DBG_ENTER("iCollision.Rectangles")
	DBG_LOCAL(t_x1,"x1")
	DBG_LOCAL(t_y1,"y1")
	DBG_LOCAL(t_width1,"width1")
	DBG_LOCAL(t_height1,"height1")
	DBG_LOCAL(t_x2,"x2")
	DBG_LOCAL(t_y2,"y2")
	DBG_LOCAL(t_width2,"width2")
	DBG_LOCAL(t_height2,"height2")
	DBG_LOCAL(t_centerRect,"centerRect")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<137>");
	if((t_centerRect)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<138>");
		t_x1=t_x1-Float(t_width1/2);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<139>");
		t_y1=t_y1-Float(t_height1/2);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<140>");
		t_x2=t_x2-Float(t_width2/2);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<141>");
		t_y2=t_y2-Float(t_height2/2);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<144>");
	if(t_x1+Float(t_width1)<=t_x2){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<145>");
	if(t_y1+Float(t_height1)<=t_y2){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<147>");
	if(t_x1>=t_x2+Float(t_width2)){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<148>");
	if(t_y1>=t_y2+Float(t_height2)){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<150>");
	return true;
}
void c_iCollision::mark(){
	Object::mark();
}
String c_iCollision::debug(){
	String t="(iCollision)\n";
	return t;
}
c_iList6::c_iList6(){
	m__length=0;
	m__data=Array<c_iEngineObject* >();
	m__index=0;
}
c_iList6* c_iList6::m_new(){
	DBG_ENTER("iList.new")
	c_iList6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>");
	return this;
}
void c_iList6::p_AddLast5(c_iEngineObject* t_value){
	DBG_ENTER("iList.AddLast")
	c_iList6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<55>");
	if(m__length==m__data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<56>");
		gc_assign(m__data,m__data.Resize(m__length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<58>");
	gc_assign(m__data.At(m__length),t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<59>");
	m__length=m__length+1;
}
int c_iList6::p_Length(){
	DBG_ENTER("iList.Length")
	c_iList6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<170>");
	return m__length;
}
c_iEngineObject* c_iList6::p_First(){
	DBG_ENTER("iList.First")
	c_iList6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>");
	m__index=0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>");
	if((m__length)!=0){
		DBG_BLOCK();
		return m__data.At(0);
	}
	return 0;
}
int c_iList6::p_Position9(c_iEngineObject* t_value){
	DBG_ENTER("iList.Position")
	c_iList6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>");
	for(int t_i=0;t_i<m__length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>");
		if(m__data.At(t_i)==t_value){
			DBG_BLOCK();
			return t_i;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>");
	return -1;
}
void c_iList6::p_RemoveFromIndex(int t_index){
	DBG_ENTER("iList.RemoveFromIndex")
	c_iList6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>");
	for(int t_i=t_index;t_i<m__length-1;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>");
		gc_assign(m__data.At(t_i),m__data.At(t_i+1));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>");
	m__length=m__length-1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>");
	m__index=m__index-1;
}
void c_iList6::p_Remove6(c_iEngineObject* t_value){
	DBG_ENTER("iList.Remove")
	c_iList6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>");
	int t_p=p_Position9(t_value);
	DBG_LOCAL(t_p,"p")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>");
	if(t_p>=0){
		DBG_BLOCK();
		p_RemoveFromIndex(t_p);
	}
}
c_iEngineObject* c_iList6::p_Ascend(){
	DBG_ENTER("iList.Ascend")
	c_iList6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>");
	m__index=m__index+1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>");
	if(m__index>=0 && m__index<m__length){
		DBG_BLOCK();
		return m__data.At(m__index);
	}
	return 0;
}
void c_iList6::mark(){
	Object::mark();
	gc_mark_q(m__data);
}
String c_iList6::debug(){
	String t="(iList)\n";
	t+=dbg_decl("_data",&m__data);
	t+=dbg_decl("_index",&m__index);
	t+=dbg_decl("_length",&m__length);
	return t;
}
int bb_graphics_DrawImage(c_Image* t_image,Float t_x,Float t_y,int t_frame){
	DBG_ENTER("DrawImage")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<452>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<453>");
	if(t_frame<0 || t_frame>=t_image->m_frames.Length()){
		DBG_BLOCK();
		bbError(String(L"Invalid image frame",19));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<456>");
	c_Frame* t_f=t_image->m_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<458>");
	bb_graphics_context->p_Validate();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<460>");
	if((t_image->m_flags&65536)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<461>");
		bb_graphics_renderDevice->DrawSurface(t_image->m_surface,t_x-t_image->m_tx,t_y-t_image->m_ty);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<463>");
		bb_graphics_renderDevice->DrawSurface2(t_image->m_surface,t_x-t_image->m_tx,t_y-t_image->m_ty,t_f->m_x,t_f->m_y,t_image->m_width,t_image->m_height);
	}
	return 0;
}
int bb_graphics_PushMatrix(){
	DBG_ENTER("PushMatrix")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<333>");
	int t_sp=bb_graphics_context->m_matrixSp;
	DBG_LOCAL(t_sp,"sp")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<334>");
	if(t_sp==bb_graphics_context->m_matrixStack.Length()){
		DBG_BLOCK();
		gc_assign(bb_graphics_context->m_matrixStack,bb_graphics_context->m_matrixStack.Resize(t_sp*2));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<335>");
	bb_graphics_context->m_matrixStack.At(t_sp+0)=bb_graphics_context->m_ix;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<336>");
	bb_graphics_context->m_matrixStack.At(t_sp+1)=bb_graphics_context->m_iy;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<337>");
	bb_graphics_context->m_matrixStack.At(t_sp+2)=bb_graphics_context->m_jx;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<338>");
	bb_graphics_context->m_matrixStack.At(t_sp+3)=bb_graphics_context->m_jy;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<339>");
	bb_graphics_context->m_matrixStack.At(t_sp+4)=bb_graphics_context->m_tx;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<340>");
	bb_graphics_context->m_matrixStack.At(t_sp+5)=bb_graphics_context->m_ty;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<341>");
	bb_graphics_context->m_matrixSp=t_sp+6;
	return 0;
}
int bb_graphics_PopMatrix(){
	DBG_ENTER("PopMatrix")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<345>");
	int t_sp=bb_graphics_context->m_matrixSp-6;
	DBG_LOCAL(t_sp,"sp")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<346>");
	bb_graphics_SetMatrix(bb_graphics_context->m_matrixStack.At(t_sp+0),bb_graphics_context->m_matrixStack.At(t_sp+1),bb_graphics_context->m_matrixStack.At(t_sp+2),bb_graphics_context->m_matrixStack.At(t_sp+3),bb_graphics_context->m_matrixStack.At(t_sp+4),bb_graphics_context->m_matrixStack.At(t_sp+5));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<347>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<470>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<471>");
	if(t_frame<0 || t_frame>=t_image->m_frames.Length()){
		DBG_BLOCK();
		bbError(String(L"Invalid image frame",19));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<474>");
	c_Frame* t_f=t_image->m_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<476>");
	bb_graphics_PushMatrix();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<478>");
	bb_graphics_Translate(t_x,t_y);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<479>");
	bb_graphics_Rotate(t_rotation);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<480>");
	bb_graphics_Scale(t_scaleX,t_scaleY);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<482>");
	bb_graphics_Translate(-t_image->m_tx,-t_image->m_ty);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<484>");
	bb_graphics_context->p_Validate();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<486>");
	if((t_image->m_flags&65536)!=0){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<487>");
		bb_graphics_renderDevice->DrawSurface(t_image->m_surface,FLOAT(0.0),FLOAT(0.0));
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<489>");
		bb_graphics_renderDevice->DrawSurface2(t_image->m_surface,FLOAT(0.0),FLOAT(0.0),t_f->m_x,t_f->m_y,t_image->m_width,t_image->m_height);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<492>");
	bb_graphics_PopMatrix();
	return 0;
}
int bb_graphics_DrawText(String t_text,Float t_x,Float t_y,Float t_xalign,Float t_yalign){
	DBG_ENTER("DrawText")
	DBG_LOCAL(t_text,"text")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_xalign,"xalign")
	DBG_LOCAL(t_yalign,"yalign")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<577>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<579>");
	if(!((bb_graphics_context->m_font)!=0)){
		DBG_BLOCK();
		return 0;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<581>");
	int t_w=bb_graphics_context->m_font->p_Width();
	DBG_LOCAL(t_w,"w")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<582>");
	int t_h=bb_graphics_context->m_font->p_Height();
	DBG_LOCAL(t_h,"h")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<584>");
	t_x-=(Float)floor(Float(t_w*t_text.Length())*t_xalign);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<585>");
	t_y-=(Float)floor(Float(t_h)*t_yalign);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<587>");
	for(int t_i=0;t_i<t_text.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<588>");
		int t_ch=(int)t_text.At(t_i)-bb_graphics_context->m_firstChar;
		DBG_LOCAL(t_ch,"ch")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<589>");
		if(t_ch>=0 && t_ch<bb_graphics_context->m_font->p_Frames()){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<590>");
			bb_graphics_DrawImage(bb_graphics_context->m_font,t_x+Float(t_i*t_w),t_y,t_ch);
		}
	}
	return 0;
}
int bb_input_KeyHit(int t_key){
	DBG_ENTER("KeyHit")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<44>");
	int t_=bb_input_device->p_KeyHit(t_key);
	return t_;
}
c_BattleScene::c_BattleScene(){
	m_enemyTimer=0;
	m_enemyTimer2=0;
	m_playerTimer=0;
	m_turn=String();
	m_playerTurns=0;
	m_bg=0;
}
c_BattleScene* c_BattleScene::m_new(){
	DBG_ENTER("BattleScene.new")
	c_BattleScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<15>");
	c_iEngine::m_new();
	return this;
}
int c_BattleScene::p_OnCreate(){
	DBG_ENTER("BattleScene.OnCreate")
	c_BattleScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<27>");
	bbPrint(String(L"Creating Battle",15));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<28>");
	m_enemyTimer=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<29>");
	m_enemyTimer2=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<30>");
	m_playerTimer=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<31>");
	m_turn=String(L"Timer",5);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<32>");
	m_playerTurns=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<34>");
	gc_assign(m_bg,bb_gfx_iLoadSprite2(String(L"cave_battle_bg.png",18),800,477,1));
	return 0;
}
int c_BattleScene::p_OnRender(){
	DBG_ENTER("BattleScene.OnRender")
	c_BattleScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<40>");
	bb_graphics_DrawImage(m_bg,FLOAT(400.0),FLOAT(240.0),0);
	return 0;
}
int c_BattleScene::p_OnStart(){
	DBG_ENTER("BattleScene.OnStart")
	c_BattleScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<46>");
	bbPrint(String(L"Starting Battle",15));
	return 0;
}
int c_BattleScene::p_OnStop(){
	DBG_ENTER("BattleScene.OnStop")
	c_BattleScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<52>");
	bbPrint(String(L"Stopping Battle",15));
	return 0;
}
int c_BattleScene::p_OnUpdate(){
	DBG_ENTER("BattleScene.OnUpdate")
	c_BattleScene *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<58>");
	if(m_playerTurns<11){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<59>");
		if(m_playerTimer>100){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<60>");
			m_turn=String(L"Player",6);
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<61>");
			if(m_enemyTimer>100){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<62>");
				m_turn=String(L"Enemy1",6);
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<63>");
				if(m_enemyTimer2>100){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<64>");
					m_turn=String(L"Enemy2",6);
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<66>");
					m_turn=String(L"Timer",5);
				}
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<69>");
		if(m_turn==String(L"Player",6)){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<73>");
			if((bb_input_KeyHit(13))!=0){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<74>");
				m_playerTimer=0;
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<75>");
				m_playerTurns+=1;
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<76>");
				if(m_playerTurns>10){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<77>");
					bbPrint(String(L"Battle Over. Testing successful!",32));
				}
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<82>");
			if(m_turn==String(L"Enemy1",6)){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<83>");
				bbPrint(String(L"Enemy1 took turn",16));
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<84>");
				m_enemyTimer=0;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<85>");
				if(m_turn==String(L"Enemy2",6)){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<86>");
					bbPrint(String(L"Enemy2 took turn",16));
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<87>");
					m_enemyTimer2=0;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<88>");
					if(m_turn==String(L"Timer",5)){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<89>");
						bbPrint(String(L"Timer turn",10));
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<90>");
						m_playerTimer+=5;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<91>");
						m_enemyTimer+=1;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<92>");
						m_enemyTimer2+=2;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<93>");
						if(m_playerTimer>100){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<94>");
							bbPrint(String(L"Player turn! Press enter to select",34));
						}
					}
				}
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<98>");
		if((bb_input_KeyHit(13))!=0){
			DBG_BLOCK();
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/battleScene.monkey<102>");
		if((bb_input_KeyHit(32))!=0){
			DBG_BLOCK();
		}
	}
	return 0;
}
void c_BattleScene::mark(){
	c_iEngine::mark();
	gc_mark_q(m_bg);
}
String c_BattleScene::debug(){
	String t="(BattleScene)\n";
	t=c_iEngine::debug()+t;
	t+=dbg_decl("enemyTimer",&m_enemyTimer);
	t+=dbg_decl("enemyTimer2",&m_enemyTimer2);
	t+=dbg_decl("playerTimer",&m_playerTimer);
	t+=dbg_decl("turn",&m_turn);
	t+=dbg_decl("playerTurns",&m_playerTurns);
	t+=dbg_decl("bg",&m_bg);
	return t;
}
c_Image* bb_gfx_iLoadSprite(String t_path,int t_frameCount){
	DBG_ENTER("iLoadSprite")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<390>");
	c_Image* t_=bb_graphics_LoadImage(t_path,t_frameCount,1);
	return t_;
}
c_Image* bb_gfx_iLoadSprite2(String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount){
	DBG_ENTER("iLoadSprite")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<399>");
	c_Image* t_=bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,1);
	return t_;
}
String bb_strings_iStripExt(String t_path){
	DBG_ENTER("iStripExt")
	DBG_LOCAL(t_path,"path")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<261>");
	int t_i=t_path.FindLast(String(L".",1));
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<262>");
	if(t_i!=-1 && t_path.Find(String(L"/",1),t_i+1)==-1 && t_path.Find(String(L"\\",1),t_i+1)==-1){
		DBG_BLOCK();
		String t_=t_path.Slice(0,t_i);
		return t_;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<264>");
	return t_path;
}
String bb_strings_iExtractExt(String t_path){
	DBG_ENTER("iExtractExt")
	DBG_LOCAL(t_path,"path")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<319>");
	int t_i=t_path.FindLast(String(L".",1));
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<320>");
	if(t_i!=-1 && t_path.Find(String(L"/",1),t_i+1)==-1 && t_path.Find(String(L"\\",1),t_i+1)==-1){
		DBG_BLOCK();
		String t_=t_path.Slice(t_i+1);
		return t_;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<321>");
	return String();
}
Array<c_Image* > bb_gfx_iLoadImage(int t_start,int t_count,String t_path,int t_frameCount,int t_flags){
	DBG_ENTER("iLoadImage")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<324>");
	int t_i=0;
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<326>");
	Array<c_Image* > t_image=Array<c_Image* >(t_count);
	DBG_LOCAL(t_image,"image")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<328>");
	String t_file=bb_strings_iStripExt(t_path);
	DBG_LOCAL(t_file,"file")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<329>");
	String t_extension=String(L".",1)+bb_strings_iExtractExt(t_path);
	DBG_LOCAL(t_extension,"extension")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<331>");
	for(int t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		DBG_BLOCK();
		DBG_LOCAL(t_c,"c")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<332>");
		if(t_c<10){
			DBG_BLOCK();
			gc_assign(t_image.At(t_i),bb_graphics_LoadImage(t_file+String(L"000",3)+String(t_c)+t_extension,t_frameCount,t_flags));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<333>");
		if(t_c>9 && t_c<100){
			DBG_BLOCK();
			gc_assign(t_image.At(t_i),bb_graphics_LoadImage(t_file+String(L"00",2)+String(t_c)+t_extension,t_frameCount,t_flags));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<334>");
		if(t_c>99 && t_c<1000){
			DBG_BLOCK();
			gc_assign(t_image.At(t_i),bb_graphics_LoadImage(t_file+String(L"0",1)+String(t_c)+t_extension,t_frameCount,t_flags));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<335>");
		t_i=t_i+1;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<338>");
	return t_image;
}
Array<c_Image* > bb_gfx_iLoadImage2(Array<c_Image* > t_image,int t_start,int t_count,String t_path,int t_frameCount,int t_flags){
	DBG_ENTER("iLoadImage")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<344>");
	int t_i=t_image.Length();
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<346>");
	t_image=t_image.Resize(t_image.Length()+t_count);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<348>");
	String t_file=bb_strings_iStripExt(t_path);
	DBG_LOCAL(t_file,"file")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<349>");
	String t_extension=String(L".",1)+bb_strings_iExtractExt(t_path);
	DBG_LOCAL(t_extension,"extension")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<351>");
	for(int t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		DBG_BLOCK();
		DBG_LOCAL(t_c,"c")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<352>");
		if(t_c<10){
			DBG_BLOCK();
			gc_assign(t_image.At(t_i),bb_graphics_LoadImage(t_file+String(L"000",3)+String(t_c)+t_extension,t_frameCount,t_flags));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<353>");
		if(t_c>9 && t_c<100){
			DBG_BLOCK();
			gc_assign(t_image.At(t_i),bb_graphics_LoadImage(t_file+String(L"00",2)+String(t_c)+t_extension,t_frameCount,t_flags));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<354>");
		if(t_c>99 && t_c<1000){
			DBG_BLOCK();
			gc_assign(t_image.At(t_i),bb_graphics_LoadImage(t_file+String(L"0",1)+String(t_c)+t_extension,t_frameCount,t_flags));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<355>");
		t_i=t_i+1;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<358>");
	return t_image;
}
Array<c_Image* > bb_gfx_iLoadImage3(int t_start,int t_count,String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount,int t_flags){
	DBG_ENTER("iLoadImage")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<369>");
	Array<c_Image* > t_i=Array<c_Image* >(t_count);
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<371>");
	String t_f=bb_strings_iStripExt(t_path);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<372>");
	String t_e=String(L".",1)+bb_strings_iExtractExt(t_path);
	DBG_LOCAL(t_e,"e")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<374>");
	for(int t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		DBG_BLOCK();
		DBG_LOCAL(t_c,"c")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<375>");
		if(t_c<10){
			DBG_BLOCK();
			gc_assign(t_i.At(t_c),bb_graphics_LoadImage2(t_f+String(L"000",3)+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<376>");
		if(t_c>9 && t_c<100){
			DBG_BLOCK();
			gc_assign(t_i.At(t_c),bb_graphics_LoadImage2(t_f+String(L"00",2)+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<377>");
		if(t_c>99 && t_c<1000){
			DBG_BLOCK();
			gc_assign(t_i.At(t_c),bb_graphics_LoadImage2(t_f+String(L"0",1)+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags));
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<380>");
	return t_i;
}
Array<c_Image* > bb_gfx_iLoadSprite3(int t_start,int t_count,String t_path,int t_frameCount){
	DBG_ENTER("iLoadSprite")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<408>");
	Array<c_Image* > t_=bb_gfx_iLoadImage(t_start,t_count,t_path,t_frameCount,1);
	return t_;
}
Array<c_Image* > bb_gfx_iLoadSprite4(int t_start,int t_count,String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount){
	DBG_ENTER("iLoadSprite")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<417>");
	Array<c_Image* > t_=bb_gfx_iLoadImage3(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,1);
	return t_;
}
c_iLayerSprite::c_iLayerSprite(){
	m__imagePointer=Array<c_Image* >(1);
	m__imageSignature=Array<c_Image* >(1);
	m__ghostImagePointer=Array<c_Image* >(1);
	m__frame=FLOAT(.0);
	m__imageIndex=FLOAT(.0);
	m__frameOffset=FLOAT(1.0);
	m__animationMode=1;
	m__imagePath=String();
}
c_iLayerSprite* c_iLayerSprite::m_new(){
	DBG_ENTER("iLayerSprite.new")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<11>");
	c_iLayerObject::m_new();
	return this;
}
Array<c_Image* > c_iLayerSprite::p_ImagePointer(){
	DBG_ENTER("iLayerSprite.ImagePointer")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<436>");
	return m__imagePointer;
}
void c_iLayerSprite::p_ImagePointer2(c_Image* t_image){
	DBG_ENTER("iLayerSprite.ImagePointer")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_image,"image")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<444>");
	if(t_image!=m__imageSignature.At(0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<446>");
		gc_assign(m__imagePointer.At(0),t_image);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<447>");
		gc_assign(m__ghostImagePointer.At(0),t_image);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<448>");
		gc_assign(m__imageSignature.At(0),t_image);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<450>");
		m__imagePointer.At(0)->p_SetHandle(Float(t_image->p_Width())*FLOAT(.5),Float(t_image->p_Height())*FLOAT(.5));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<452>");
		m__frame=FLOAT(0.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<453>");
		m__imageIndex=FLOAT(0.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<455>");
		m__frameOffset=FLOAT(1.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<456>");
		m__animationMode=1;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<458>");
		if(m__imagePointer.Length()>1){
			DBG_BLOCK();
			gc_assign(m__imagePointer,m__imagePointer.Resize(1));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<459>");
		if(m__ghostImagePointer.Length()>1){
			DBG_BLOCK();
			gc_assign(m__ghostImagePointer,m__ghostImagePointer.Resize(1));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<460>");
		if(m__imageSignature.Length()>1){
			DBG_BLOCK();
			gc_assign(m__imageSignature,m__imageSignature.Resize(1));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<462>");
		m__imagePath=String();
	}
}
bool c_iLayerSprite::p_Compare2(Array<c_Image* > t_image1,Array<c_Image* > t_image2){
	DBG_ENTER("iLayerSprite.Compare")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_image1,"image1")
	DBG_LOCAL(t_image2,"image2")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<229>");
	if(t_image1.Length()!=t_image2.Length()){
		DBG_BLOCK();
		return false;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<231>");
	for(int t_i=0;t_i<t_image1.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<232>");
		if(t_image1.At(t_i)!=t_image2.At(t_i)){
			DBG_BLOCK();
			return false;
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<235>");
	return true;
}
void c_iLayerSprite::p_ImagePointer3(Array<c_Image* > t_image){
	DBG_ENTER("iLayerSprite.ImagePointer")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_image,"image")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<473>");
	if(p_Compare2(t_image,m__imageSignature)==false){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<475>");
		gc_assign(m__imagePointer,m__imagePointer.Resize(t_image.Length()));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<476>");
		for(int t_i=0;t_i<t_image.Length();t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<477>");
			gc_assign(m__imagePointer.At(t_i),t_image.At(t_i));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<480>");
		gc_assign(m__ghostImagePointer,m__ghostImagePointer.Resize(t_image.Length()));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<481>");
		for(int t_i2=0;t_i2<t_image.Length();t_i2=t_i2+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i2,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<482>");
			gc_assign(m__ghostImagePointer.At(t_i2),t_image.At(t_i2));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<485>");
		gc_assign(m__imageSignature,m__imageSignature.Resize(t_image.Length()));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<486>");
		for(int t_i3=0;t_i3<t_image.Length();t_i3=t_i3+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i3,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<487>");
			gc_assign(m__imageSignature.At(t_i3),t_image.At(t_i3));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<490>");
		for(int t_i4=0;t_i4<m__imagePointer.Length();t_i4=t_i4+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i4,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<491>");
			m__imagePointer.At(t_i4)->p_SetHandle(Float(m__imagePointer.At(t_i4)->p_Width())*FLOAT(.5),Float(m__imagePointer.At(t_i4)->p_Height())*FLOAT(.5));
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<494>");
		m__frame=FLOAT(0.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<495>");
		m__imageIndex=FLOAT(0.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<497>");
		m__frameOffset=FLOAT(1.0);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<498>");
		m__animationMode=1;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<500>");
		m__imagePath=String();
	}
}
void c_iLayerSprite::p_ImagePointer4(int t_start,int t_count,String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount){
	DBG_ENTER("iLayerSprite.ImagePointer")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<575>");
	if(t_path!=m__imagePath){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<577>");
		Array<c_Image* > t_image=bb_contentmanager_iContent->p_GetImage4(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,c_Image::m_DefaultFlags);
		DBG_LOCAL(t_image,"image")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<579>");
		p_ImagePointer3(t_image);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<580>");
		m__imagePath=t_path;
	}
}
void c_iLayerSprite::p_ImagePointer5(int t_start,int t_count,String t_path,int t_frameCount){
	DBG_ENTER("iLayerSprite.ImagePointer")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<555>");
	if(t_path!=m__imagePath){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<557>");
		Array<c_Image* > t_image=bb_contentmanager_iContent->p_GetImage3(t_start,t_count,t_path,t_frameCount,c_Image::m_DefaultFlags);
		DBG_LOCAL(t_image,"image")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<559>");
		p_ImagePointer3(t_image);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<560>");
		m__imagePath=t_path;
	}
}
void c_iLayerSprite::p_ImagePointer6(String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount){
	DBG_ENTER("iLayerSprite.ImagePointer")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<535>");
	if(t_path!=m__imagePath){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<537>");
		c_Image* t_image=bb_contentmanager_iContent->p_GetImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,c_Image::m_DefaultFlags);
		DBG_LOCAL(t_image,"image")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<539>");
		p_ImagePointer2(t_image);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<540>");
		m__imagePath=t_path;
	}
}
void c_iLayerSprite::p_ImagePointer7(String t_path,int t_frameCount){
	DBG_ENTER("iLayerSprite.ImagePointer")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<515>");
	if(t_path!=m__imagePath){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<517>");
		c_Image* t_image=bb_contentmanager_iContent->p_GetImage(t_path,t_frameCount,c_Image::m_DefaultFlags);
		DBG_LOCAL(t_image,"image")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<519>");
		p_ImagePointer2(t_image);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<520>");
		m__imagePath=t_path;
	}
}
void c_iLayerSprite::p_Destroy(){
	DBG_ENTER("iLayerSprite.Destroy")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<280>");
	c_iLayerObject::p_Destroy();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<282>");
	m__imagePointer=Array<c_Image* >();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<283>");
	m__imageSignature=Array<c_Image* >();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<284>");
	m__ghostImagePointer=Array<c_Image* >();
}
Float c_iLayerSprite::p_ImageIndex(){
	DBG_ENTER("iLayerSprite.ImageIndex")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<335>");
	Float t_=(Float)fmod(m__imageIndex,Float(m__imagePointer.Length()));
	return t_;
}
void c_iLayerSprite::p_ImageIndex2(Float t_index){
	DBG_ENTER("iLayerSprite.ImageIndex")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<342>");
	m__imageIndex=t_index;
}
bool c_iLayerSprite::p_ImageLoaded(){
	DBG_ENTER("iLayerSprite.ImageLoaded")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<429>");
	if((m__imagePointer.At(int(p_ImageIndex())))!=0){
		DBG_BLOCK();
		return true;
	}else{
		DBG_BLOCK();
		return false;
	}
}
Float c_iLayerSprite::p_Height(){
	DBG_ENTER("iLayerSprite.Height")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<416>");
	if(p_ImageLoaded()){
		DBG_BLOCK();
		Float t_=bb_math2_Abs2(Float(m__imagePointer.At(int(p_ImageIndex()))->p_Height())*m__scaleY);
		return t_;
	}else{
		DBG_BLOCK();
		return FLOAT(0.0);
	}
}
Float c_iLayerSprite::p_ImageFrame(){
	DBG_ENTER("iLayerSprite.ImageFrame")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<421>");
	Float t_=(Float)fmod(m__frame,Float(m__imagePointer.At(int(p_ImageIndex()))->p_Frames()));
	return t_;
}
void c_iLayerSprite::p_Render(){
	DBG_ENTER("iLayerSprite.Render")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<659>");
	if(!p_ImageLoaded()){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<661>");
	if(m__ghost){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<664>");
		if(bb_graphics_GetBlend()!=m__ghostBlend){
			DBG_BLOCK();
			bb_graphics_SetBlend(m__ghostBlend);
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<668>");
		p_SetAlpha(m__ghostAlpha);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<672>");
		p_SetColor(m__ghostRed,m__ghostGreen,m__ghostBlue);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<675>");
		bb_graphics_DrawImage2(m__ghostImagePointer.At(int(p_ImageIndex())),p_RenderX(),p_RenderY(),p_Rotation()*Float(bb_globals_iRotation),p_ScaleX(),p_ScaleY(),int(p_ImageFrame()));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<677>");
		m__ghost=false;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<682>");
		if(bb_graphics_GetBlend()!=m__blend){
			DBG_BLOCK();
			bb_graphics_SetBlend(m__blend);
		}
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<686>");
		p_SetAlpha(m__alpha);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<690>");
		p_SetColor(m__red,m__green,m__blue);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<693>");
		bb_graphics_DrawImage2(m__imagePointer.At(int(p_ImageIndex())),p_RenderX(),p_RenderY(),p_Rotation()*Float(bb_globals_iRotation),p_ScaleX(),p_ScaleY(),int(p_ImageFrame()));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<697>");
	p_RenderDebugInfo();
}
void c_iLayerSprite::p_Update(){
	DBG_ENTER("iLayerSprite.Update")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<747>");
	p_ClearDebugInfo();
}
Float c_iLayerSprite::p_Width(){
	DBG_ENTER("iLayerSprite.Width")
	c_iLayerSprite *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<754>");
	if(p_ImageLoaded()){
		DBG_BLOCK();
		Float t_=bb_math2_Abs2(Float(m__imagePointer.At(int(p_ImageIndex()))->p_Width())*m__scaleX);
		return t_;
	}else{
		DBG_BLOCK();
		return FLOAT(0.0);
	}
}
void c_iLayerSprite::mark(){
	c_iLayerObject::mark();
	gc_mark_q(m__imagePointer);
	gc_mark_q(m__imageSignature);
	gc_mark_q(m__ghostImagePointer);
}
String c_iLayerSprite::debug(){
	String t="(iLayerSprite)\n";
	t=c_iLayerObject::debug()+t;
	t+=dbg_decl("_animationMode",&m__animationMode);
	t+=dbg_decl("_frame",&m__frame);
	t+=dbg_decl("_frameOffset",&m__frameOffset);
	t+=dbg_decl("_imageIndex",&m__imageIndex);
	t+=dbg_decl("_imagePath",&m__imagePath);
	t+=dbg_decl("_imagePointer",&m__imagePointer);
	t+=dbg_decl("_imageSignature",&m__imageSignature);
	t+=dbg_decl("_ghostImagePointer",&m__ghostImagePointer);
	return t;
}
c_iLayerGroup::c_iLayerGroup(){
	m__objectList=(new c_iList3)->m_new();
}
void c_iLayerGroup::p_AddItemLast(c_iLayerObject* t_layerObject){
	DBG_ENTER("iLayerGroup.AddItemLast")
	c_iLayerGroup *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_layerObject,"layerObject")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<45>");
	m__objectList->p_AddLast4(t_layerObject);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<47>");
	t_layerObject->p_Layer2(p_Layer());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<49>");
	t_layerObject->p_Parent2(this);
}
Float c_iLayerGroup::p_Height(){
	DBG_ENTER("iLayerGroup.Height")
	c_iLayerGroup *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<122>");
	Float t_=Float(m__height)*m__scaleY;
	return t_;
}
void c_iLayerGroup::p_Render(){
	DBG_ENTER("iLayerGroup.Render")
	c_iLayerGroup *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<177>");
	p_RenderList(m__objectList);
}
void c_iLayerGroup::p_Update(){
	DBG_ENTER("iLayerGroup.Update")
	c_iLayerGroup *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<245>");
	p_UpdateList(m__objectList);
}
Float c_iLayerGroup::p_Width(){
	DBG_ENTER("iLayerGroup.Width")
	c_iLayerGroup *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<269>");
	Float t_=Float(m__width)*m__scaleX;
	return t_;
}
void c_iLayerGroup::p_UpdateWorldXY(){
	DBG_ENTER("iLayerGroup.UpdateWorldXY")
	c_iLayerGroup *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<253>");
	m__worldHeight=bb_math2_Abs2(p_Height());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<254>");
	m__worldScaleX=p_ScaleX();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<255>");
	m__worldScaleY=p_ScaleX();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<256>");
	m__worldRotation=p_Rotation();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<257>");
	m__worldWidth=bb_math2_Abs2(p_Width());
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<258>");
	m__worldX=m__x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<259>");
	m__worldY=m__y;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<261>");
	p_UpdateWorldXYList(m__objectList);
}
void c_iLayerGroup::mark(){
	c_iLayerObject::mark();
	gc_mark_q(m__objectList);
}
String c_iLayerGroup::debug(){
	String t="(iLayerGroup)\n";
	t=c_iLayerObject::debug()+t;
	t+=dbg_decl("_objectList",&m__objectList);
	return t;
}
c_iContentManager::c_iContentManager(){
	m__data=Array<c_iContentObject* >(128);
	m__length=0;
	m__cache=0;
}
void c_iContentManager::p_SystemInit(){
	DBG_ENTER("iContentManager.SystemInit")
	c_iContentManager *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<525>");
	for(int t_i=0;t_i<m__data.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<526>");
		gc_assign(m__data.At(t_i),(new c_iContentObject)->m_new());
	}
}
c_iContentManager* c_iContentManager::m_new(){
	DBG_ENTER("iContentManager.new")
	c_iContentManager *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<476>");
	p_SystemInit();
	return this;
}
void c_iContentManager::p_Expand(){
	DBG_ENTER("iContentManager.Expand")
	c_iContentManager *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<380>");
	if(m__length==m__data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<382>");
		gc_assign(m__data,m__data.Resize(m__length*2+10));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<384>");
		for(int t_i=0;t_i<m__data.Length();t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<385>");
			if(!((m__data.At(t_i))!=0)){
				DBG_BLOCK();
				gc_assign(m__data.At(t_i),(new c_iContentObject)->m_new());
			}
		}
	}
}
c_iContentObject* c_iContentManager::p_Get2(String t_path){
	DBG_ENTER("iContentManager.Get")
	c_iContentManager *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_path,"path")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<70>");
	for(m__cache=0;m__cache<m__length;m__cache=m__cache+1){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<71>");
		if(m__data.At(m__cache)->m__path==t_path){
			DBG_BLOCK();
			return m__data.At(m__cache);
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<73>");
	return 0;
}
c_Image* c_iContentManager::p_GetImage(String t_path,int t_frameCount,int t_flags){
	DBG_ENTER("iContentManager.GetImage")
	c_iContentManager *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<160>");
	if(m__length>0 && ((m__data.At(m__cache))!=0) && m__data.At(m__cache)->m__path==t_path){
		DBG_BLOCK();
		return m__data.At(m__cache)->m__imagePointer;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<162>");
	p_Expand();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<164>");
	c_iContentObject* t_o=p_Get2(t_path);
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<166>");
	if(((t_o)!=0) && ((t_o->m__imagePointer)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<168>");
		return t_o->m__imagePointer;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<172>");
		t_o=m__data.At(m__length);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<174>");
		m__length=m__length+1;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<176>");
		gc_assign(t_o->m__imagePointer,bb_graphics_LoadImage(t_path,t_frameCount,t_flags));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<178>");
		t_o->m__path=t_path;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<180>");
		return t_o->m__imagePointer;
	}
}
c_Image* c_iContentManager::p_GetImage2(String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount,int t_flags){
	DBG_ENTER("iContentManager.GetImage")
	c_iContentManager *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<205>");
	if(m__length>0 && ((m__data.At(m__cache))!=0) && m__data.At(m__cache)->m__path==t_path){
		DBG_BLOCK();
		return m__data.At(m__cache)->m__imagePointer;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<207>");
	p_Expand();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<209>");
	c_iContentObject* t_o=p_Get2(t_path);
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<211>");
	if(((t_o)!=0) && ((t_o->m__imagePointer)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<213>");
		return t_o->m__imagePointer;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<217>");
		t_o=m__data.At(m__length);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<219>");
		m__length=m__length+1;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<221>");
		gc_assign(t_o->m__imagePointer,bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<223>");
		t_o->m__path=t_path;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<225>");
		return t_o->m__imagePointer;
	}
}
Array<c_Image* > c_iContentManager::p_GetImage3(int t_start,int t_count,String t_path,int t_frameCount,int t_flags){
	DBG_ENTER("iContentManager.GetImage")
	c_iContentManager *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<242>");
	if(m__length>0 && ((m__data.At(m__cache))!=0) && m__data.At(m__cache)->m__path==t_path){
		DBG_BLOCK();
		return m__data.At(m__cache)->m__imagePointers;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<244>");
	p_Expand();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<246>");
	c_iContentObject* t_o=p_Get2(t_path);
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<248>");
	if(((t_o)!=0) && ((t_o->m__imagePointers.At(0))!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<250>");
		return t_o->m__imagePointers;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<254>");
		t_o=m__data.At(m__length);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<256>");
		m__length=m__length+1;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<258>");
		gc_assign(t_o->m__imagePointers,bb_gfx_iLoadImage(t_start,t_count,t_path,t_frameCount,t_flags));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<260>");
		t_o->m__path=t_path;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<262>");
		return t_o->m__imagePointers;
	}
}
Array<c_Image* > c_iContentManager::p_GetImage4(int t_start,int t_count,String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount,int t_flags){
	DBG_ENTER("iContentManager.GetImage")
	c_iContentManager *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_start,"start")
	DBG_LOCAL(t_count,"count")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<279>");
	if(m__length>0 && ((m__data.At(m__cache))!=0) && m__data.At(m__cache)->m__path==t_path){
		DBG_BLOCK();
		return m__data.At(m__cache)->m__imagePointers;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<281>");
	p_Expand();
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<283>");
	c_iContentObject* t_o=p_Get2(t_path);
	DBG_LOCAL(t_o,"o")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<285>");
	if(((t_o)!=0) && ((t_o->m__imagePointers.At(0))!=0)){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<287>");
		return t_o->m__imagePointers;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<291>");
		t_o=m__data.At(m__length);
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<293>");
		m__length=m__length+1;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<295>");
		gc_assign(t_o->m__imagePointers,bb_gfx_iLoadImage3(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags));
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<297>");
		t_o->m__path=t_path;
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<299>");
		return t_o->m__imagePointers;
	}
}
void c_iContentManager::mark(){
	Object::mark();
	gc_mark_q(m__data);
}
String c_iContentManager::debug(){
	String t="(iContentManager)\n";
	t+=dbg_decl("_cache",&m__cache);
	t+=dbg_decl("_data",&m__data);
	t+=dbg_decl("_length",&m__length);
	return t;
}
c_iContentObject::c_iContentObject(){
	m__path=String();
	m__imagePointer=0;
	m__imagePointers=Array<c_Image* >();
}
c_iContentObject* c_iContentObject::m_new(){
	DBG_ENTER("iContentObject.new")
	c_iContentObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<546>");
	return this;
}
void c_iContentObject::mark(){
	Object::mark();
	gc_mark_q(m__imagePointer);
	gc_mark_q(m__imagePointers);
}
String c_iContentObject::debug(){
	String t="(iContentObject)\n";
	t+=dbg_decl("_imagePointer",&m__imagePointer);
	t+=dbg_decl("_imagePointers",&m__imagePointers);
	t+=dbg_decl("_path",&m__path);
	return t;
}
c_iContentManager* bb_contentmanager_iContent;
c_Player::c_Player(){
	m_image=0;
	m_x=0;
	m_y=0;
	m_xVel=0;
	m_yVel=0;
	m_downAnim=0;
	m_currentAnimation=0;
}
c_Player* c_Player::m_new(c_Image* t_i,int t_x,int t_y){
	DBG_ENTER("Player.new")
	c_Player *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_i,"i")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<21>");
	gc_assign(this->m_image,t_i);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<22>");
	this->m_x=t_x;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<23>");
	this->m_y=t_y;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<24>");
	this->m_xVel=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<25>");
	this->m_yVel=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<26>");
	gc_assign(this->m_downAnim,(new c_Animation)->m_new(t_i,4,69,102,200));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<27>");
	gc_assign(this->m_currentAnimation,this->m_downAnim);
	return this;
}
c_Player* c_Player::m_new2(){
	DBG_ENTER("Player.new")
	c_Player *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<7>");
	return this;
}
void c_Player::mark(){
	Object::mark();
	gc_mark_q(m_image);
	gc_mark_q(m_downAnim);
	gc_mark_q(m_currentAnimation);
}
String c_Player::debug(){
	String t="(Player)\n";
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	t+=dbg_decl("xVel",&m_xVel);
	t+=dbg_decl("yVel",&m_yVel);
	t+=dbg_decl("image",&m_image);
	t+=dbg_decl("downAnim",&m_downAnim);
	t+=dbg_decl("currentAnimation",&m_currentAnimation);
	return t;
}
c_Animation::c_Animation(){
	m_img=0;
	m_frames=0;
	m_width=0;
	m_height=0;
	m_frameTime=0;
	m_elapsed=0;
	m_frame=0;
	m_lastTime=0;
}
c_Animation* c_Animation::m_new(c_Image* t_i,int t_f,int t_w,int t_h,int t_ft){
	DBG_ENTER("Animation.new")
	c_Animation *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_i,"i")
	DBG_LOCAL(t_f,"f")
	DBG_LOCAL(t_w,"w")
	DBG_LOCAL(t_h,"h")
	DBG_LOCAL(t_ft,"ft")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<20>");
	gc_assign(this->m_img,t_i);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<21>");
	this->m_frames=t_f;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<22>");
	this->m_width=t_w;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<23>");
	this->m_height=t_h;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<24>");
	this->m_frameTime=t_ft;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<26>");
	this->m_elapsed=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<27>");
	this->m_frame=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<28>");
	this->m_lastTime=0;
	return this;
}
c_Animation* c_Animation::m_new2(){
	DBG_ENTER("Animation.new")
	c_Animation *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<6>");
	return this;
}
void c_Animation::mark(){
	Object::mark();
	gc_mark_q(m_img);
}
String c_Animation::debug(){
	String t="(Animation)\n";
	t+=dbg_decl("img",&m_img);
	t+=dbg_decl("frames",&m_frames);
	t+=dbg_decl("frame",&m_frame);
	t+=dbg_decl("width",&m_width);
	t+=dbg_decl("height",&m_height);
	t+=dbg_decl("frameTime",&m_frameTime);
	t+=dbg_decl("elapsed",&m_elapsed);
	t+=dbg_decl("lastTime",&m_lastTime);
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<32>");
	gc_assign(this->m_sample,t_sample);
	return this;
}
c_Sound* c_Sound::m_new2(){
	DBG_ENTER("Sound.new")
	c_Sound *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<29>");
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
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<47>");
	gxtkSample* t_sample=bb_audio_device->LoadSample(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_sample,"sample")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<48>");
	if((t_sample)!=0){
		DBG_BLOCK();
		c_Sound* t_=(new c_Sound)->m_new(t_sample);
		return t_;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<49>");
	return 0;
}
c_Level::c_Level(){
	m_layout=Array<Array<int > >();
	m_generated=false;
	m_xCoord=0;
	m_yCoord=0;
	m_width=0;
	m_height=0;
	m_walkways=Array<c_Point* >();
	m_lava=Array<c_Point* >();
	m_counter=0;
	m_entranceX=0;
	m_entranceY=0;
	m_treasureX=0;
	m_treasureY=0;
	m_treasures=0;
}
int c_Level::p_Draw(int t_xOffsetG,int t_yOffsetG){
	DBG_ENTER("Level.Draw")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_xOffsetG,"xOffsetG")
	DBG_LOCAL(t_yOffsetG,"yOffsetG")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<671>");
	int t_xTile=t_xOffsetG/40-1;
	DBG_LOCAL(t_xTile,"xTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<672>");
	int t_yTile=t_yOffsetG/40-1;
	DBG_LOCAL(t_yTile,"yTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<673>");
	int t_xOffset=t_xOffsetG % 40;
	DBG_LOCAL(t_xOffset,"xOffset")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<674>");
	int t_yOffset=t_yOffsetG % 40;
	DBG_LOCAL(t_yOffset,"yOffset")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<676>");
	for(int t_i=t_xTile;t_i<t_xTile+18;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<677>");
		for(int t_j=t_yTile;t_j<t_yTile+14;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<678>");
			if(t_i>-1 && t_i<m_layout.Length() && t_j>-1 && t_j<m_layout.At(t_i).Length()){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<679>");
				bb_graphics_DrawImage(bb_level_caveTextures,Float(t_i*40),Float(t_j*40),m_layout.At(t_i).At(t_j));
			}
		}
	}
	return 0;
}
Array<Array<int > > c_Level::p_setArray(int t_i,int t_j){
	DBG_ENTER("Level.setArray")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_i,"i")
	DBG_LOCAL(t_j,"j")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<476>");
	Array<Array<int > > t_result=Array<Array<int > >(t_i);
	DBG_LOCAL(t_result,"result")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<478>");
	for(int t_index=0;t_index<t_i;t_index=t_index+1){
		DBG_BLOCK();
		DBG_LOCAL(t_index,"index")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<479>");
		gc_assign(t_result.At(t_index),Array<int >(t_j));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<482>");
	return t_result;
}
int c_Level::p_randomlyAssignCells(Array<Array<int > > t_design){
	DBG_ENTER("Level.randomlyAssignCells")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_design,"design")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<489>");
	bb_random_Seed=bb_app_Millisecs();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<490>");
	int t_rand=0;
	DBG_LOCAL(t_rand,"rand")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<492>");
	for(int t_i=0;t_i<t_design.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<493>");
		for(int t_j=0;t_j<t_design.At(0).Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<494>");
			t_rand=int(bb_random_Rnd2(FLOAT(0.0),FLOAT(100.0)));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<495>");
			if(t_rand<45){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<496>");
				t_design.At(t_i).At(t_j)=0;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<498>");
				t_design.At(t_i).At(t_j)=5;
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<500>");
			if(t_i==0 || t_j==0 || t_i==t_design.Length()-1 || t_j==t_design.At(0).Length()-1){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<501>");
				t_design.At(t_i).At(t_j)=0;
			}
		}
	}
	return 0;
}
int c_Level::p_checkWalls(Array<Array<int > > t_design,int t_i,int t_j){
	DBG_ENTER("Level.checkWalls")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_design,"design")
	DBG_LOCAL(t_i,"i")
	DBG_LOCAL(t_j,"j")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<546>");
	int t_total=0;
	DBG_LOCAL(t_total,"total")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<547>");
	if(t_i>0 && t_design.At(t_i-1).At(t_j)==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<548>");
		t_total+=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<550>");
	if(t_i<t_design.Length()-1 && t_design.At(t_i+1).At(t_j)==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<551>");
		t_total+=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<553>");
	if(t_j>0 && t_design.At(t_i).At(t_j-1)==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<554>");
		t_total+=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<556>");
	if(t_j<t_design.At(0).Length()-1 && t_design.At(t_i).At(t_j+1)==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<557>");
		t_total+=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<559>");
	if(t_i>0 && t_j<t_design.At(0).Length()-1 && t_design.At(t_i-1).At(t_j+1)==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<560>");
		t_total+=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<562>");
	if(t_i>0 && t_j>0 && t_design.At(t_i-1).At(t_j-1)==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<563>");
		t_total+=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<565>");
	if(t_i<t_design.Length()-1 && t_j<t_design.At(0).Length()-1 && t_design.At(t_i+1).At(t_j+1)==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<566>");
		t_total+=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<568>");
	if(t_i<t_design.Length()-1 && t_j>0 && t_design.At(t_i+1).At(t_j-1)==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<569>");
		t_total+=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<572>");
	return t_total;
}
Array<Array<int > > c_Level::p_generateCellularly(Array<Array<int > > t_design){
	DBG_ENTER("Level.generateCellularly")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_design,"design")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<513>");
	int t_adjacentWalls=0;
	DBG_LOCAL(t_adjacentWalls,"adjacentWalls")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<514>");
	Array<Array<int > > t_result=p_setArray(t_design.Length(),t_design.At(0).Length());
	DBG_LOCAL(t_result,"result")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<516>");
	for(int t_i=0;t_i<t_design.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<517>");
		for(int t_j=0;t_j<t_design.At(0).Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<518>");
			t_adjacentWalls=p_checkWalls(t_design,t_i,t_j);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<520>");
			if(t_design.At(t_i).At(t_j)==0){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<521>");
				if(t_adjacentWalls>3){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<522>");
					t_result.At(t_i).At(t_j)=0;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<524>");
					t_result.At(t_i).At(t_j)=5;
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<526>");
				if(t_design.At(t_i).At(t_j)==5){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<527>");
					if(t_adjacentWalls>4){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<528>");
						t_result.At(t_i).At(t_j)=0;
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<530>");
						t_result.At(t_i).At(t_j)=5;
					}
				}
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<534>");
			if(t_i==0 || t_j==0 || t_i==t_design.Length()-1 || t_j==t_design.At(0).Length()-1){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<535>");
				t_result.At(t_i).At(t_j)=0;
			}
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<540>");
	return t_result;
}
int c_Level::p_smoothEdges(){
	DBG_ENTER("Level.smoothEdges")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<419>");
	Array<Array<int > > t_cave=this->m_layout;
	DBG_LOCAL(t_cave,"cave")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<420>");
	bool t_up=false;
	DBG_LOCAL(t_up,"up")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<421>");
	bool t_down=false;
	DBG_LOCAL(t_down,"down")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<422>");
	bool t_left=false;
	DBG_LOCAL(t_left,"left")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<423>");
	bool t_right=false;
	DBG_LOCAL(t_right,"right")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<425>");
	for(int t_i=0;t_i<t_cave.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<426>");
		for(int t_j=0;t_j<t_cave.At(0).Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<427>");
			t_up=false;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<428>");
			t_down=false;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<429>");
			t_left=false;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<430>");
			t_right=false;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<432>");
			if(t_cave.At(t_i).At(t_j)==5){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<433>");
				if(t_i>0 && t_cave.At(t_i-1).At(t_j)==0){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<434>");
					t_up=true;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<436>");
				if(t_i<t_cave.Length()-2 && t_cave.At(t_i+1).At(t_j)==0){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<437>");
					t_down=true;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<439>");
				if(t_j>0 && t_cave.At(t_i).At(t_j-1)==0){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<440>");
					t_left=true;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<442>");
				if(t_j<t_cave.At(0).Length()-2 && t_cave.At(t_i).At(t_j+1)==0){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<443>");
					t_right=true;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<446>");
				if(t_up==true && t_down==false){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<447>");
					if(t_left==true && t_right==false){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<448>");
						t_cave.At(t_i).At(t_j)=1;
					}else{
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<449>");
						if(t_right==true && t_left==false){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<450>");
							t_cave.At(t_i).At(t_j)=3;
						}
					}
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<452>");
					if(t_down==true && t_up==false){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<453>");
						if(t_left==true && t_right==false){
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<454>");
							t_cave.At(t_i).At(t_j)=2;
						}else{
							DBG_BLOCK();
							DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<455>");
							if(t_right==true && t_left==false){
								DBG_BLOCK();
								DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<456>");
								t_cave.At(t_i).At(t_j)=4;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
int c_Level::p_fillCells(Array<Array<int > > t_design){
	DBG_ENTER("Level.fillCells")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_design,"design")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<593>");
	for(int t_i=0;t_i<t_design.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<594>");
		for(int t_j=0;t_j<t_design.At(0).Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<595>");
			t_design.At(t_i).At(t_j)=0;
		}
	}
	return 0;
}
int c_Level::p_drunkWalk(Array<Array<int > > t_design){
	DBG_ENTER("Level.drunkWalk")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_design,"design")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<602>");
	Float t_target=Float(t_design.Length()*t_design.At(0).Length())*FLOAT(0.35);
	DBG_LOCAL(t_target,"target")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<603>");
	int t_cleared=1;
	DBG_LOCAL(t_cleared,"cleared")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<605>");
	int t_tempX=0;
	DBG_LOCAL(t_tempX,"tempX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<606>");
	int t_tempY=0;
	DBG_LOCAL(t_tempY,"tempY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<607>");
	int t_direction=0;
	DBG_LOCAL(t_direction,"direction")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<609>");
	int t_followBias=0;
	DBG_LOCAL(t_followBias,"followBias")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<610>");
	t_tempX=int(bb_random_Rnd2(FLOAT(1.0),Float(t_design.Length()-1)));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<611>");
	t_tempY=int(bb_random_Rnd2(FLOAT(1.0),Float(t_design.At(0).Length()-1)));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<613>");
	t_design.At(t_tempX).At(t_tempY)=5;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<615>");
	while(Float(t_cleared)<t_target){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<616>");
		if(t_followBias<40){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<617>");
			t_direction=int(bb_random_Rnd2(FLOAT(0.0),FLOAT(4.0)));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<619>");
		if(t_direction==0 && t_tempY>1){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<620>");
			t_tempY-=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<621>");
			if(t_direction==1 && t_tempY<t_design.At(0).Length()-2){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<622>");
				t_tempY+=1;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<623>");
				if(t_direction==2 && t_tempX>1){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<624>");
					t_tempX-=1;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<625>");
					if(t_direction==3 && t_tempX<t_design.Length()-2){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<626>");
						t_tempX+=1;
					}
				}
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<629>");
		if(t_design.At(t_tempX).At(t_tempY)==0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<630>");
			t_design.At(t_tempX).At(t_tempY)=5;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<631>");
			t_cleared+=1;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<633>");
		t_followBias=int(bb_random_Rnd2(FLOAT(0.0),FLOAT(100.0)));
	}
	return 0;
}
int c_Level::p_countWalkways(){
	DBG_ENTER("Level.countWalkways")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<397>");
	gc_assign(this->m_walkways,Array<c_Point* >(1));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<399>");
	int t_index=0;
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<400>");
	for(int t_i=0;t_i<m_layout.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<401>");
		for(int t_j=0;t_j<m_layout.At(0).Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<402>");
			if(m_layout.At(t_i).At(t_j)==5){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<403>");
				gc_assign(this->m_walkways.At(t_index),(new c_Point)->m_new(t_i,t_j));
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<404>");
				gc_assign(this->m_walkways,this->m_walkways.Resize(t_index+2));
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<405>");
				t_index+=1;
			}
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<410>");
	gc_assign(this->m_walkways,this->m_walkways.Resize(this->m_walkways.Length()-1));
	return 0;
}
int c_Level::p_makeLavaRiver(int t_startX,int t_startY){
	DBG_ENTER("Level.makeLavaRiver")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_startX,"startX")
	DBG_LOCAL(t_startY,"startY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<330>");
	int t_currentX=t_startX;
	DBG_LOCAL(t_currentX,"currentX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<331>");
	int t_currentY=t_startY;
	DBG_LOCAL(t_currentY,"currentY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<332>");
	int t_nextX=t_startX;
	DBG_LOCAL(t_nextX,"nextX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<333>");
	int t_nextY=t_startY;
	DBG_LOCAL(t_nextY,"nextY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<338>");
	int t_randDirection=0;
	DBG_LOCAL(t_randDirection,"randDirection")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<339>");
	bool t_riverEnd=false;
	DBG_LOCAL(t_riverEnd,"riverEnd")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<340>");
	int t_riverLength=0;
	DBG_LOCAL(t_riverLength,"riverLength")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<342>");
	int t_up=0;
	DBG_LOCAL(t_up,"up")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<343>");
	int t_right=1;
	DBG_LOCAL(t_right,"right")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<344>");
	int t_down=2;
	DBG_LOCAL(t_down,"down")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<345>");
	int t_left=3;
	DBG_LOCAL(t_left,"left")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<348>");
	int t_previousDirection=-1;
	DBG_LOCAL(t_previousDirection,"previousDirection")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<349>");
	int t_direction=0;
	DBG_LOCAL(t_direction,"direction")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<351>");
	int t_riverType=0;
	DBG_LOCAL(t_riverType,"riverType")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<353>");
	while(t_riverEnd==false && t_riverLength<20){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<354>");
		t_direction=int(bb_random_Rnd2(FLOAT(0.0),FLOAT(4.0)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<355>");
		if(t_direction==t_up){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<356>");
			t_nextY-=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<357>");
			if(t_direction==t_down){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<358>");
				t_nextY+=1;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<359>");
				if(t_direction==t_left){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<360>");
					t_nextX-=1;
				}else{
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<361>");
					if(t_direction==t_right){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<362>");
						t_nextX+=1;
					}
				}
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<365>");
		if(m_layout.At(t_nextX).At(t_nextY)==0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<366>");
			int t_rng=int(bb_random_Rnd2(FLOAT(0.0),FLOAT(100.0)));
			DBG_LOCAL(t_rng,"rng")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<367>");
			if(t_rng<10){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<368>");
				t_riverEnd=true;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<370>");
				t_nextX=t_currentX;
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<371>");
				t_nextY=t_currentY;
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<374>");
		m_layout.At(t_currentX).At(t_currentY)=12;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<375>");
		gc_assign(m_lava.At(this->m_counter),(new c_Point)->m_new(t_currentX,t_currentY));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<376>");
		this->m_counter+=1;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<378>");
		if(this->m_counter==m_lava.Length()){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<379>");
			gc_assign(m_lava,m_lava.Resize(m_lava.Length()+1000));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<386>");
		t_currentX=t_nextX;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<387>");
		t_currentY=t_nextY;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<388>");
		t_riverLength+=1;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<389>");
		t_previousDirection=t_direction;
	}
	return 0;
}
int c_Level::p_makeLavaRivers(){
	DBG_ENTER("Level.makeLavaRivers")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<305>");
	this->m_counter=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<306>");
	int t_total=m_walkways.Length()/200;
	DBG_LOCAL(t_total,"total")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<309>");
	int t_randX=0;
	DBG_LOCAL(t_randX,"randX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<310>");
	int t_randY=0;
	DBG_LOCAL(t_randY,"randY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<311>");
	int t_randTile=0;
	DBG_LOCAL(t_randTile,"randTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<313>");
	int t_index=0;
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<314>");
	while(t_index<t_total){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<315>");
		t_randX=int(bb_random_Rnd2(FLOAT(0.0),Float(this->m_width)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<316>");
		t_randY=int(bb_random_Rnd2(FLOAT(0.0),Float(this->m_height)));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<317>");
		t_randTile=m_layout.At(t_randX).At(t_randY);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<318>");
		if(t_randTile==5){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<319>");
			if(m_layout.At(t_randX-1).At(t_randY)==0 || m_layout.At(t_randX+1).At(t_randY)==0 || m_layout.At(t_randX).At(t_randY-1)==0 || m_layout.At(t_randX).At(t_randY+1)==0){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<320>");
				p_makeLavaRiver(t_randX,t_randY);
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<322>");
				t_index+=1;
			}
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<326>");
	gc_assign(m_lava,m_lava.Resize(m_counter));
	return 0;
}
int c_Level::p_addSpikesAndTraps(){
	DBG_ENTER("Level.addSpikesAndTraps")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<154>");
	int t_spikeCount=m_walkways.Length()/30;
	DBG_LOCAL(t_spikeCount,"spikeCount")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<155>");
	int t_counter=0;
	DBG_LOCAL(t_counter,"counter")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<156>");
	int t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_walkways.Length())));
	DBG_LOCAL(t_randTile,"randTile")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<158>");
	int t_x=0;
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<159>");
	int t_y=0;
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<160>");
	while(t_counter<t_spikeCount){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<161>");
		t_x=m_walkways.At(t_randTile)->p_getX();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<162>");
		t_y=m_walkways.At(t_randTile)->p_getY();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<163>");
		if(m_layout.At(t_x).At(t_y)==5){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<164>");
			m_layout.At(t_x).At(t_y)=15;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<165>");
			t_counter+=1;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<167>");
		t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_walkways.Length())));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<170>");
	int t_trapCount=m_walkways.Length()/50;
	DBG_LOCAL(t_trapCount,"trapCount")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<171>");
	t_counter=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<172>");
	t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_walkways.Length())));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<174>");
	t_x=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<175>");
	t_y=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<176>");
	while(t_counter<t_trapCount){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<177>");
		t_x=m_walkways.At(t_randTile)->p_getX();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<178>");
		t_y=m_walkways.At(t_randTile)->p_getY();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<179>");
		if(m_layout.At(t_x).At(t_y)==5){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<180>");
			m_layout.At(t_x).At(t_y)=13;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<181>");
			t_counter+=1;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<183>");
		t_randTile=int(bb_random_Rnd2(FLOAT(0.0),Float(m_walkways.Length())));
	}
	return 0;
}
int c_Level::p_makeStartingPoint(){
	DBG_ENTER("Level.makeStartingPoint")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<189>");
	int t_rng=int(bb_random_Rnd2(FLOAT(0.0),Float(m_walkways.Length())));
	DBG_LOCAL(t_rng,"rng")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<190>");
	int t_isSet=0;
	DBG_LOCAL(t_isSet,"isSet")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<192>");
	int t_x=0;
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<193>");
	int t_y=0;
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<197>");
	while(!((t_isSet)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<199>");
		t_x=m_walkways.At(t_rng)->p_getX();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<200>");
		t_y=m_walkways.At(t_rng)->p_getY();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<202>");
		if(m_layout.At(t_x).At(t_y)==5){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<203>");
			t_isSet=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<205>");
			t_rng=int(bb_random_Rnd2(FLOAT(0.0),Float(m_walkways.Length())));
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<210>");
	m_layout.At(t_x).At(t_y)=16;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<211>");
	m_entranceX=t_x;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<212>");
	m_entranceY=t_y;
	return 0;
}
int c_Level::p_setTreasure(){
	DBG_ENTER("Level.setTreasure")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<219>");
	int t_rng=int(bb_random_Rnd2(FLOAT(0.0),Float(m_walkways.Length())));
	DBG_LOCAL(t_rng,"rng")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<220>");
	int t_isSet=0;
	DBG_LOCAL(t_isSet,"isSet")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<222>");
	int t_x=0;
	DBG_LOCAL(t_x,"x")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<223>");
	int t_y=0;
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<224>");
	while(!((t_isSet)!=0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<225>");
		t_x=m_walkways.At(t_rng)->p_getX();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<226>");
		t_y=m_walkways.At(t_rng)->p_getY();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<228>");
		if(m_layout.At(t_x).At(t_y)==5){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<229>");
			t_isSet=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<231>");
			t_rng=int(bb_random_Rnd2(FLOAT(0.0),Float(m_walkways.Length())));
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<235>");
	m_layout.At(t_x).At(t_y)=17;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<236>");
	this->m_treasureX=t_x;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<237>");
	this->m_treasureY=t_y;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<238>");
	m_treasures->p_Push10((new c_Point)->m_new(t_x,t_y));
	return 0;
}
bool c_Level::p_setTreasure2(int t_total){
	DBG_ENTER("Level.setTreasure")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_total,"total")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<115>");
	int t_successCount=0;
	DBG_LOCAL(t_successCount,"successCount")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<116>");
	int t_failCount=0;
	DBG_LOCAL(t_failCount,"failCount")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<117>");
	c_AStarSearch* t_searcher=(new c_AStarSearch)->m_new(this->m_layout,300,false);
	DBG_LOCAL(t_searcher,"searcher")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<118>");
	c_Path* t_currentPath=0;
	DBG_LOCAL(t_currentPath,"currentPath")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<120>");
	while(t_successCount<10 && t_failCount<10){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<121>");
		p_setTreasure();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<122>");
		int t_closestIndex=-1;
		DBG_LOCAL(t_closestIndex,"closestIndex")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<123>");
		Float t_closestDist=bb_level_getDist(m_entranceX,m_entranceY,m_treasures->p_Get(m_treasures->p_Length()-1)->p_getX(),m_treasures->p_Get(m_treasures->p_Length()-1)->p_getY());
		DBG_LOCAL(t_closestDist,"closestDist")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<124>");
		for(int t_t=0;t_t<m_treasures->p_Length();t_t=t_t+1){
			DBG_BLOCK();
			DBG_LOCAL(t_t,"t")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<125>");
			Float t_localDist=bb_level_getDist(m_treasures->p_Get(m_treasures->p_Length()-1)->p_getX(),m_treasures->p_Get(m_treasures->p_Length()-1)->p_getY(),m_treasures->p_Get(t_t)->p_getX(),m_treasures->p_Get(t_t)->p_getY());
			DBG_LOCAL(t_localDist,"localDist")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<126>");
			if(t_localDist<t_closestDist){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<127>");
				t_closestDist=t_localDist;
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<128>");
				t_closestIndex=t_t;
			}
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<131>");
		if(t_closestIndex==-1){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<132>");
			t_currentPath=t_searcher->p_findPath(m_entranceX,m_entranceY,m_treasures->p_Get(m_treasures->p_Length()-1)->p_getX(),m_treasures->p_Get(m_treasures->p_Length()-1)->p_getY());
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<134>");
			t_currentPath=t_searcher->p_findPath(m_treasures->p_Get(t_closestIndex)->p_getX(),m_treasures->p_Get(t_closestIndex)->p_getY(),m_treasures->p_Get(m_treasures->p_Length()-1)->p_getX(),m_treasures->p_Get(m_treasures->p_Length()-1)->p_getY());
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<137>");
		if(t_currentPath==0){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<138>");
			t_failCount+=1;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<139>");
			m_treasures->p_Pop();
		}else{
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<141>");
			t_successCount+=1;
		}
	}
	return false;
}
c_Level* c_Level::m_new(int t_x,int t_y,int t_w,int t_h,String t_type){
	DBG_ENTER("Level.new")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_w,"w")
	DBG_LOCAL(t_h,"h")
	DBG_LOCAL(t_type,"type")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<67>");
	this->m_generated=false;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<68>");
	this->m_xCoord=t_x;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<69>");
	this->m_yCoord=t_y;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<71>");
	this->m_width=t_w;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<72>");
	this->m_height=t_h;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<73>");
	gc_assign(this->m_layout,p_setArray(this->m_width,this->m_height));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<75>");
	gc_assign(bb_level_caveTextures,bb_gfx_iLoadSprite2(String(L"cave_texture40.png",18),40,40,19));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<77>");
	if(t_type==String(L"Cellular",8)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<78>");
		p_randomlyAssignCells(this->m_layout);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<79>");
		gc_assign(m_layout,p_generateCellularly(this->m_layout));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<80>");
		gc_assign(m_layout,p_generateCellularly(this->m_layout));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<81>");
		gc_assign(m_layout,p_generateCellularly(this->m_layout));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<82>");
		gc_assign(m_layout,p_generateCellularly(this->m_layout));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<83>");
		gc_assign(m_layout,p_generateCellularly(this->m_layout));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<84>");
		p_smoothEdges();
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<85>");
		if(t_type==String(L"Drunk",5)){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<86>");
			p_fillCells(this->m_layout);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<87>");
			p_drunkWalk(this->m_layout);
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<90>");
	p_countWalkways();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<91>");
	gc_assign(m_lava,Array<c_Point* >(1000));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<92>");
	p_makeLavaRivers();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<94>");
	p_addSpikesAndTraps();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<96>");
	p_makeStartingPoint();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<98>");
	this->m_treasureX=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<99>");
	this->m_treasureY=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<101>");
	p_setTreasure2(10);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<111>");
	this->m_generated=true;
	return this;
}
c_Level* c_Level::m_new2(){
	DBG_ENTER("Level.new")
	c_Level *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<13>");
	return this;
}
void c_Level::mark(){
	Object::mark();
	gc_mark_q(m_layout);
	gc_mark_q(m_walkways);
	gc_mark_q(m_lava);
	gc_mark_q(m_treasures);
}
String c_Level::debug(){
	String t="(Level)\n";
	t+=dbg_decl("layout",&m_layout);
	t+=dbg_decl("width",&m_width);
	t+=dbg_decl("height",&m_height);
	t+=dbg_decl("generated",&m_generated);
	t+=dbg_decl("walkways",&m_walkways);
	t+=dbg_decl("lava",&m_lava);
	t+=dbg_decl("counter",&m_counter);
	t+=dbg_decl("xCoord",&m_xCoord);
	t+=dbg_decl("yCoord",&m_yCoord);
	t+=dbg_decl("treasureX",&m_treasureX);
	t+=dbg_decl("treasureY",&m_treasureY);
	t+=dbg_decl("treasures",&m_treasures);
	t+=dbg_decl("entranceX",&m_entranceX);
	t+=dbg_decl("entranceY",&m_entranceY);
	return t;
}
c_Image* bb_level_caveTextures;
Float bb_random_Rnd(){
	DBG_ENTER("Rnd")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/random.monkey<21>");
	bb_random_Seed=bb_random_Seed*1664525+1013904223|0;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/random.monkey<22>");
	Float t_=Float(bb_random_Seed>>8&16777215)/FLOAT(16777216.0);
	return t_;
}
Float bb_random_Rnd2(Float t_low,Float t_high){
	DBG_ENTER("Rnd")
	DBG_LOCAL(t_low,"low")
	DBG_LOCAL(t_high,"high")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/random.monkey<30>");
	Float t_=bb_random_Rnd3(t_high-t_low)+t_low;
	return t_;
}
Float bb_random_Rnd3(Float t_range){
	DBG_ENTER("Rnd")
	DBG_LOCAL(t_range,"range")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/random.monkey<26>");
	Float t_=bb_random_Rnd()*t_range;
	return t_;
}
c_Point::c_Point(){
	m_x=0;
	m_y=0;
}
c_Point* c_Point::m_new(int t_x,int t_y){
	DBG_ENTER("Point.new")
	c_Point *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/point.monkey<11>");
	this->m_x=t_x;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/point.monkey<12>");
	this->m_y=t_y;
	return this;
}
c_Point* c_Point::m_new2(){
	DBG_ENTER("Point.new")
	c_Point *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/point.monkey<6>");
	return this;
}
int c_Point::p_getX(){
	DBG_ENTER("Point.getX")
	c_Point *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/point.monkey<16>");
	return this->m_x;
}
int c_Point::p_getY(){
	DBG_ENTER("Point.getY")
	c_Point *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/point.monkey<20>");
	return this->m_y;
}
void c_Point::mark(){
	Object::mark();
}
String c_Point::debug(){
	String t="(Point)\n";
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	return t;
}
c_AStarSearch::c_AStarSearch(){
	m_gameMap=Array<Array<int > >();
	m_maxDistance=0;
	m_allowDiagonal=false;
	m_visited=0;
	m_unvisited=0;
	m_nodes=0;
}
c_AStarSearch* c_AStarSearch::m_new(Array<Array<int > > t_gMap,int t_maxDist,bool t_allowDiag){
	DBG_ENTER("AStarSearch.new")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_gMap,"gMap")
	DBG_LOCAL(t_maxDist,"maxDist")
	DBG_LOCAL(t_allowDiag,"allowDiag")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<25>");
	gc_assign(this->m_gameMap,t_gMap);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<26>");
	this->m_maxDistance=t_maxDist;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<27>");
	this->m_allowDiagonal=t_allowDiag;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<29>");
	gc_assign(this->m_visited,(new c_Stack2)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<30>");
	gc_assign(this->m_unvisited,(new c_Stack2)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<32>");
	gc_assign(this->m_nodes,(new c_Stack3)->m_new());
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<33>");
	int t_idCount=0;
	DBG_LOCAL(t_idCount,"idCount")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<34>");
	for(int t_i=0;t_i<m_gameMap.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<35>");
		this->m_nodes->p_Push7((new c_Stack2)->m_new());
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<36>");
		for(int t_j=0;t_j<m_gameMap.At(t_i).Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<37>");
			this->m_nodes->p_Get(t_i)->p_Push4((new c_AStarNode)->m_new(t_i,t_j,t_idCount));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<38>");
			t_idCount+=1;
		}
	}
	return this;
}
c_AStarSearch* c_AStarSearch::m_new2(){
	DBG_ENTER("AStarSearch.new")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<12>");
	return this;
}
int c_AStarSearch::p_isWalkableTile(Array<Array<int > > t_gameMap,int t_x,int t_y){
	DBG_ENTER("AStarSearch.isWalkableTile")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_gameMap,"gameMap")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<367>");
	int t_=((t_gameMap.At(t_x).At(t_y)==5 || t_gameMap.At(t_x).At(t_y)==17)?1:0);
	return t_;
}
c_AStarNode* c_AStarSearch::p_getLowestCostNode(c_Stack2* t_n){
	DBG_ENTER("AStarSearch.getLowestCostNode")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_n,"n")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<60>");
	int t_index=0;
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<62>");
	Float t_lowestCost=t_n->p_Get(0)->m_totalCost;
	DBG_LOCAL(t_lowestCost,"lowestCost")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<64>");
	for(int t_i=1;t_i<t_n->p_Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<65>");
		if(t_n->p_Get(t_i)->m_totalCost<t_lowestCost){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<66>");
			t_index=t_i;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<67>");
			t_lowestCost=t_n->p_Get(t_i)->m_totalCost;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<71>");
	c_AStarNode* t_=t_n->p_Get(t_index);
	return t_;
}
c_AStarNode* c_AStarSearch::p_getLowestCostNode2(){
	DBG_ENTER("AStarSearch.getLowestCostNode")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<76>");
	int t_index=0;
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<78>");
	Float t_lowestCost=this->m_unvisited->p_Get(0)->m_totalCost;
	DBG_LOCAL(t_lowestCost,"lowestCost")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<80>");
	for(int t_i=1;t_i<this->m_unvisited->p_Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<81>");
		if(this->m_unvisited->p_Get(t_i)->m_totalCost<t_lowestCost){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<82>");
			t_index=t_i;
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<83>");
			t_lowestCost=this->m_unvisited->p_Get(t_i)->m_totalCost;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<87>");
	c_AStarNode* t_=this->m_unvisited->p_Get(t_index);
	return t_;
}
bool c_AStarSearch::p_remIfStackHasElement(c_Stack2* t_s,c_AStarNode* t_n){
	DBG_ENTER("AStarSearch.remIfStackHasElement")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_s,"s")
	DBG_LOCAL(t_n,"n")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<45>");
	for(int t_i=0;t_i<t_s->p_Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<47>");
		if(t_s->p_Get(t_i)->m_id==t_n->m_id){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<49>");
			t_s->p_Remove7(t_i);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<51>");
			return true;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<55>");
	return false;
}
c_Path* c_AStarSearch::p_findPath(int t_sx,int t_sy,int t_tx,int t_ty){
	DBG_ENTER("AStarSearch.findPath")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_sx,"sx")
	DBG_LOCAL(t_sy,"sy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<92>");
	if(!((p_isWalkableTile(this->m_gameMap,t_tx,t_ty))!=0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<93>");
		bbPrint(String(L"Treasure not walkable",21));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<94>");
		return 0;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<97>");
	for(int t_i=0;t_i<this->m_nodes->p_Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<98>");
		for(int t_j=0;t_j<this->m_nodes->p_Get(t_i)->p_Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<99>");
			this->m_nodes->p_Get(t_i)->p_Get(t_j)->p_setCost(t_i,t_j,t_tx,t_ty);
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<103>");
	c_AStarNode* t_startNode=this->m_nodes->p_Get(t_sx)->p_Get(t_sy);
	DBG_LOCAL(t_startNode,"startNode")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<104>");
	c_AStarNode* t_endNode=this->m_nodes->p_Get(t_tx)->p_Get(t_ty);
	DBG_LOCAL(t_endNode,"endNode")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<106>");
	t_startNode->m_cost=FLOAT(0.0);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<107>");
	t_startNode->m_depth=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<109>");
	this->m_visited->p_Clear();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<110>");
	this->m_unvisited->p_Clear();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<112>");
	this->m_unvisited->p_Push4(t_startNode);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<114>");
	t_endNode->m_parent=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<116>");
	int t_depth=0;
	DBG_LOCAL(t_depth,"depth")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<117>");
	int t_iterations=0;
	DBG_LOCAL(t_iterations,"iterations")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<119>");
	while(t_depth<300 && m_unvisited->p_Length()>0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<120>");
		t_iterations+=1;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<121>");
		c_AStarNode* t_currentNode=p_getLowestCostNode2();
		DBG_LOCAL(t_currentNode,"currentNode")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<123>");
		if(t_currentNode->m_x==t_sx-1 && t_currentNode->m_y==t_sy){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<124>");
			bbPrint(String(L"Starting left neighbor visited!: Iteration ",43)+String(t_iterations));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<126>");
		if(t_currentNode->m_x==t_sx+1 && t_currentNode->m_y==t_sy){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<127>");
			bbPrint(String(L"Starting right neighbor visited!: Iteration ",44)+String(t_iterations));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<130>");
		if(t_currentNode->m_x==t_sx && t_currentNode->m_y==t_sy-1){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<131>");
			bbPrint(String(L"Starting up neighbor visited!: Iteration ",41)+String(t_iterations));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<134>");
		if(t_currentNode->m_x==t_sx && t_currentNode->m_y==t_sy+1){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<135>");
			bbPrint(String(L"Starting down neighbor visited!: Iteration ",43)+String(t_iterations));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<139>");
		if(t_endNode->m_id==t_currentNode->m_id){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<140>");
			break;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<143>");
		p_remIfStackHasElement(this->m_unvisited,t_currentNode);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<150>");
		this->m_visited->p_Push4(t_currentNode);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<152>");
		for(int t_i2=-1;t_i2<2;t_i2=t_i2+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i2,"i")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<153>");
			for(int t_j2=-1;t_j2<2;t_j2=t_j2+1){
				DBG_BLOCK();
				DBG_LOCAL(t_j2,"j")
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<154>");
				if(t_i2==0 && t_j2==0){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<155>");
					continue;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<158>");
				if(!this->m_allowDiagonal){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<159>");
					if(((!((t_i2)!=0))?1:0)==0 && ((!((t_j2)!=0))?1:0)==0){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<160>");
						continue;
					}
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<164>");
				int t_xpos=t_i2+t_currentNode->m_x;
				DBG_LOCAL(t_xpos,"xpos")
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<165>");
				int t_ypos=t_j2+t_currentNode->m_y;
				DBG_LOCAL(t_ypos,"ypos")
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<167>");
				if((p_isWalkableTile(this->m_gameMap,t_xpos,t_ypos))!=0){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<169>");
					c_AStarNode* t_neighbor=this->m_nodes->p_Get(t_xpos)->p_Get(t_ypos);
					DBG_LOCAL(t_neighbor,"neighbor")
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<172>");
					Float t_nextStepCost=t_currentNode->m_cost+FLOAT(1.0);
					DBG_LOCAL(t_nextStepCost,"nextStepCost")
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<174>");
					if(t_nextStepCost<t_neighbor->m_cost){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<175>");
						p_remIfStackHasElement(this->m_unvisited,t_neighbor);
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<176>");
						p_remIfStackHasElement(this->m_visited,t_neighbor);
					}
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<179>");
					if(!bb_pathfinder_stackHasElement(this->m_unvisited,t_neighbor) && !bb_pathfinder_stackHasElement(this->m_visited,t_neighbor)){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<180>");
						t_neighbor->m_cost=t_nextStepCost;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<182>");
						t_neighbor->m_totalCost=t_nextStepCost+t_neighbor->m_heuristic;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<184>");
						t_depth=bb_math2_Max(t_depth,t_neighbor->p_setParent(t_currentNode));
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<185>");
						this->m_unvisited->p_Push4(t_neighbor);
					}
				}
			}
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<194>");
	if(t_endNode->m_parent==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<195>");
		bbPrint(String(L"End node id: ",13)+String(t_endNode->m_id));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<196>");
		bbPrint(String(L"End node null",13));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<197>");
		return 0;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<200>");
	bbPrint(String(L"Iterations to treasure: ",24)+String(t_iterations));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<202>");
	c_Path* t_foundPath=(new c_Path)->m_new();
	DBG_LOCAL(t_foundPath,"foundPath")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<203>");
	c_AStarNode* t_target=this->m_nodes->p_Get(t_tx)->p_Get(t_ty);
	DBG_LOCAL(t_target,"target")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<205>");
	while(!(t_target->m_x==t_startNode->m_x && t_target->m_y==t_startNode->m_y)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<206>");
		t_foundPath->p_prependStep(t_target->m_x,t_target->m_y);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<207>");
		t_target=t_target->m_parent;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<209>");
	t_foundPath->p_prependStep(t_sx,t_sy);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<211>");
	bbPrint(String(L"Path Length: ",13)+String(t_foundPath->p_pathLength()));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<214>");
	return t_foundPath;
}
bool c_AStarSearch::p_isWalkableTileOverworld(Array<Array<int > > t_m,int t_x,int t_y){
	DBG_ENTER("AStarSearch.isWalkableTileOverworld")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_m,"m")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<219>");
	int t_i=t_m.At(t_x).At(t_y);
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<221>");
	if(t_i==0 || t_i==8 || t_i==9 || t_i==10 || t_i==11 || t_i==21 || t_i==22){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<222>");
		return false;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<224>");
		return true;
	}
}
int c_AStarSearch::p_getTileCostOverworld(Array<Array<int > > t_m,int t_x,int t_y){
	DBG_ENTER("AStarSearch.getTileCostOverworld")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_m,"m")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<357>");
	int t_i=t_m.At(t_x).At(t_y);
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<358>");
	if(t_i>22 && t_i<30 || t_i>11 && t_i<18){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<359>");
		return 10;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<361>");
		return 1;
	}
}
c_Path* c_AStarSearch::p_findPathOverworld(Array<Array<int > > t_overworld,int t_sx,int t_sy,int t_tx,int t_ty){
	DBG_ENTER("AStarSearch.findPathOverworld")
	c_AStarSearch *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_overworld,"overworld")
	DBG_LOCAL(t_sx,"sx")
	DBG_LOCAL(t_sy,"sy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<230>");
	if(!p_isWalkableTileOverworld(this->m_gameMap,t_tx,t_ty)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<231>");
		bbPrint(String(L"Cave tile not walkable",22));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<232>");
		return 0;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<235>");
	for(int t_i=0;t_i<this->m_nodes->p_Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<236>");
		for(int t_j=0;t_j<this->m_nodes->p_Get(t_i)->p_Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<237>");
			this->m_nodes->p_Get(t_i)->p_Get(t_j)->p_setCost(t_i,t_j,t_tx,t_ty);
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<241>");
	c_AStarNode* t_startNode=this->m_nodes->p_Get(t_sx)->p_Get(t_sy);
	DBG_LOCAL(t_startNode,"startNode")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<242>");
	c_AStarNode* t_endNode=this->m_nodes->p_Get(t_tx)->p_Get(t_ty);
	DBG_LOCAL(t_endNode,"endNode")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<244>");
	t_startNode->m_cost=FLOAT(0.0);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<245>");
	t_startNode->m_depth=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<247>");
	this->m_visited->p_Clear();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<248>");
	this->m_unvisited->p_Clear();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<250>");
	this->m_unvisited->p_Push4(t_startNode);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<252>");
	t_endNode->m_parent=0;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<254>");
	int t_depth=0;
	DBG_LOCAL(t_depth,"depth")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<255>");
	int t_iterations=0;
	DBG_LOCAL(t_iterations,"iterations")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<257>");
	while(t_depth<300 && m_unvisited->p_Length()>0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<258>");
		t_iterations+=1;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<259>");
		c_AStarNode* t_currentNode=p_getLowestCostNode2();
		DBG_LOCAL(t_currentNode,"currentNode")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<261>");
		if(t_currentNode->m_x==t_sx-1 && t_currentNode->m_y==t_sy){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<262>");
			bbPrint(String(L"Starting left neighbor visited!: Iteration ",43)+String(t_iterations));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<264>");
		if(t_currentNode->m_x==t_sx+1 && t_currentNode->m_y==t_sy){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<265>");
			bbPrint(String(L"Starting right neighbor visited!: Iteration ",44)+String(t_iterations));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<268>");
		if(t_currentNode->m_x==t_sx && t_currentNode->m_y==t_sy-1){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<269>");
			bbPrint(String(L"Starting up neighbor visited!: Iteration ",41)+String(t_iterations));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<272>");
		if(t_currentNode->m_x==t_sx && t_currentNode->m_y==t_sy+1){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<273>");
			bbPrint(String(L"Starting down neighbor visited!: Iteration ",43)+String(t_iterations));
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<277>");
		if(t_endNode->m_id==t_currentNode->m_id){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<278>");
			break;
		}
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<281>");
		p_remIfStackHasElement(this->m_unvisited,t_currentNode);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<288>");
		this->m_visited->p_Push4(t_currentNode);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<290>");
		for(int t_i2=-1;t_i2<2;t_i2=t_i2+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i2,"i")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<291>");
			for(int t_j2=-1;t_j2<2;t_j2=t_j2+1){
				DBG_BLOCK();
				DBG_LOCAL(t_j2,"j")
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<292>");
				if(t_i2==0 && t_j2==0){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<293>");
					continue;
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<296>");
				if(!this->m_allowDiagonal){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<297>");
					if(((!((t_i2)!=0))?1:0)==0 && ((!((t_j2)!=0))?1:0)==0){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<298>");
						continue;
					}
				}
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<302>");
				int t_xpos=t_i2+t_currentNode->m_x;
				DBG_LOCAL(t_xpos,"xpos")
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<303>");
				int t_ypos=t_j2+t_currentNode->m_y;
				DBG_LOCAL(t_ypos,"ypos")
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<305>");
				if(p_isWalkableTileOverworld(this->m_gameMap,t_xpos,t_ypos)){
					DBG_BLOCK();
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<307>");
					c_AStarNode* t_neighbor=this->m_nodes->p_Get(t_xpos)->p_Get(t_ypos);
					DBG_LOCAL(t_neighbor,"neighbor")
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<310>");
					Float t_nextStepCost=t_currentNode->m_cost+Float(p_getTileCostOverworld(this->m_gameMap,t_xpos,t_ypos));
					DBG_LOCAL(t_nextStepCost,"nextStepCost")
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<312>");
					if(t_nextStepCost<t_neighbor->m_cost){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<313>");
						p_remIfStackHasElement(this->m_unvisited,t_neighbor);
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<314>");
						p_remIfStackHasElement(this->m_visited,t_neighbor);
					}
					DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<317>");
					if(!bb_pathfinder_stackHasElement(this->m_unvisited,t_neighbor) && !bb_pathfinder_stackHasElement(this->m_visited,t_neighbor)){
						DBG_BLOCK();
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<318>");
						t_neighbor->m_cost=t_nextStepCost;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<320>");
						t_neighbor->m_totalCost=t_nextStepCost+t_neighbor->m_heuristic;
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<322>");
						t_depth=bb_math2_Max(t_depth,t_neighbor->p_setParent(t_currentNode));
						DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<323>");
						this->m_unvisited->p_Push4(t_neighbor);
					}
				}
			}
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<332>");
	if(t_endNode->m_parent==0){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<333>");
		bbPrint(String(L"End node id: ",13)+String(t_endNode->m_id));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<334>");
		bbPrint(String(L"End node null",13));
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<335>");
		return 0;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<338>");
	bbPrint(String(L"Iterations to treasure: ",24)+String(t_iterations));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<340>");
	c_Path* t_foundPath=(new c_Path)->m_new();
	DBG_LOCAL(t_foundPath,"foundPath")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<341>");
	c_AStarNode* t_target=this->m_nodes->p_Get(t_tx)->p_Get(t_ty);
	DBG_LOCAL(t_target,"target")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<343>");
	while(!(t_target->m_x==t_startNode->m_x && t_target->m_y==t_startNode->m_y)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<344>");
		t_foundPath->p_prependStep(t_target->m_x,t_target->m_y);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<345>");
		t_target=t_target->m_parent;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<347>");
	t_foundPath->p_prependStep(t_sx,t_sy);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<349>");
	bbPrint(String(L"Path Length: ",13)+String(t_foundPath->p_pathLength()));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<352>");
	return t_foundPath;
}
void c_AStarSearch::mark(){
	Object::mark();
	gc_mark_q(m_gameMap);
	gc_mark_q(m_visited);
	gc_mark_q(m_unvisited);
	gc_mark_q(m_nodes);
}
String c_AStarSearch::debug(){
	String t="(AStarSearch)\n";
	t+=dbg_decl("visited",&m_visited);
	t+=dbg_decl("unvisited",&m_unvisited);
	t+=dbg_decl("gameMap",&m_gameMap);
	t+=dbg_decl("maxDistance",&m_maxDistance);
	t+=dbg_decl("nodes",&m_nodes);
	t+=dbg_decl("allowDiagonal",&m_allowDiagonal);
	return t;
}
c_AStarNode::c_AStarNode(){
	m_x=0;
	m_y=0;
	m_id=0;
	m_cost=FLOAT(.0);
	m_totalCost=FLOAT(.0);
	m_heuristic=FLOAT(.0);
	m_depth=0;
	m_parent=0;
}
c_AStarNode* c_AStarNode::m_new(int t_xCoord,int t_yCoord,int t_nodeID){
	DBG_ENTER("AStarNode.new")
	c_AStarNode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_xCoord,"xCoord")
	DBG_LOCAL(t_yCoord,"yCoord")
	DBG_LOCAL(t_nodeID,"nodeID")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<385>");
	this->m_x=t_xCoord;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<386>");
	this->m_y=t_yCoord;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<387>");
	this->m_id=t_nodeID;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<388>");
	this->m_cost=FLOAT(200000.0);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<389>");
	this->m_totalCost=m_cost;
	return this;
}
c_AStarNode* c_AStarNode::m_new2(){
	DBG_ENTER("AStarNode.new")
	c_AStarNode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<371>");
	return this;
}
int c_AStarNode::p_setCost(int t_x,int t_y,int t_tx,int t_ty){
	DBG_ENTER("AStarNode.setCost")
	c_AStarNode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<401>");
	this->m_heuristic=bb_pathfinder_getCost(t_x,t_y,t_tx,t_ty)*FLOAT(2.0);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<402>");
	this->m_totalCost=this->m_cost+this->m_heuristic;
	return 0;
}
int c_AStarNode::p_setParent(c_AStarNode* t_mamaNode){
	DBG_ENTER("AStarNode.setParent")
	c_AStarNode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_mamaNode,"mamaNode")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<393>");
	this->m_depth=t_mamaNode->m_depth+1;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<394>");
	gc_assign(this->m_parent,t_mamaNode);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<396>");
	return this->m_depth;
}
void c_AStarNode::mark(){
	Object::mark();
	gc_mark_q(m_parent);
}
String c_AStarNode::debug(){
	String t="(AStarNode)\n";
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	t+=dbg_decl("id",&m_id);
	t+=dbg_decl("parent",&m_parent);
	t+=dbg_decl("heuristic",&m_heuristic);
	t+=dbg_decl("totalCost",&m_totalCost);
	t+=dbg_decl("cost",&m_cost);
	t+=dbg_decl("depth",&m_depth);
	return t;
}
c_Stack2::c_Stack2(){
	m_data=Array<c_AStarNode* >();
	m_length=0;
}
c_Stack2* c_Stack2::m_new(){
	DBG_ENTER("Stack.new")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
c_Stack2* c_Stack2::m_new2(Array<c_AStarNode* > t_data){
	DBG_ENTER("Stack.new")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<13>");
	gc_assign(this->m_data,t_data.Slice(0));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<14>");
	this->m_length=t_data.Length();
	return this;
}
void c_Stack2::p_Push4(c_AStarNode* t_value){
	DBG_ENTER("Stack.Push")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<71>");
	if(m_length==m_data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<72>");
		gc_assign(m_data,m_data.Resize(m_length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<74>");
	gc_assign(m_data.At(m_length),t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<75>");
	m_length+=1;
}
void c_Stack2::p_Push5(Array<c_AStarNode* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<83>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<84>");
		p_Push4(t_values.At(t_offset+t_i));
	}
}
void c_Stack2::p_Push6(Array<c_AStarNode* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<79>");
	p_Push5(t_values,t_offset,t_values.Length()-t_offset);
}
c_AStarNode* c_Stack2::m_NIL;
void c_Stack2::p_Length2(int t_newlength){
	DBG_ENTER("Stack.Length")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_newlength,"newlength")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<45>");
	if(t_newlength<m_length){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<46>");
		for(int t_i=t_newlength;t_i<m_length;t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<47>");
			gc_assign(m_data.At(t_i),m_NIL);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<49>");
		if(t_newlength>m_data.Length()){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<50>");
			gc_assign(m_data,m_data.Resize(bb_math2_Max(m_length*2+10,t_newlength)));
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<52>");
	m_length=t_newlength;
}
int c_Stack2::p_Length(){
	DBG_ENTER("Stack.Length")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<56>");
	return m_length;
}
c_AStarNode* c_Stack2::p_Get(int t_index){
	DBG_ENTER("Stack.Get")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<104>");
	return m_data.At(t_index);
}
void c_Stack2::p_Clear(){
	DBG_ENTER("Stack.Clear")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<34>");
	for(int t_i=0;t_i<m_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<35>");
		gc_assign(m_data.At(t_i),m_NIL);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<37>");
	m_length=0;
}
void c_Stack2::p_Remove7(int t_index){
	DBG_ENTER("Stack.Remove")
	c_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<137>");
	for(int t_i=t_index;t_i<m_length-1;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<138>");
		gc_assign(m_data.At(t_i),m_data.At(t_i+1));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<140>");
	m_length-=1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<141>");
	gc_assign(m_data.At(m_length),m_NIL);
}
void c_Stack2::mark(){
	Object::mark();
	gc_mark_q(m_data);
}
String c_Stack2::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("NIL",&c_Stack2::m_NIL);
	t+=dbg_decl("data",&m_data);
	t+=dbg_decl("length",&m_length);
	return t;
}
c_Stack3::c_Stack3(){
	m_data=Array<c_Stack2* >();
	m_length=0;
}
c_Stack3* c_Stack3::m_new(){
	DBG_ENTER("Stack.new")
	c_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
c_Stack3* c_Stack3::m_new2(Array<c_Stack2* > t_data){
	DBG_ENTER("Stack.new")
	c_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<13>");
	gc_assign(this->m_data,t_data.Slice(0));
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<14>");
	this->m_length=t_data.Length();
	return this;
}
void c_Stack3::p_Push7(c_Stack2* t_value){
	DBG_ENTER("Stack.Push")
	c_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<71>");
	if(m_length==m_data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<72>");
		gc_assign(m_data,m_data.Resize(m_length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<74>");
	gc_assign(m_data.At(m_length),t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<75>");
	m_length+=1;
}
void c_Stack3::p_Push8(Array<c_Stack2* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	c_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<83>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<84>");
		p_Push7(t_values.At(t_offset+t_i));
	}
}
void c_Stack3::p_Push9(Array<c_Stack2* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	c_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<79>");
	p_Push8(t_values,t_offset,t_values.Length()-t_offset);
}
c_Stack2* c_Stack3::p_Get(int t_index){
	DBG_ENTER("Stack.Get")
	c_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<104>");
	return m_data.At(t_index);
}
c_Stack2* c_Stack3::m_NIL;
void c_Stack3::p_Length2(int t_newlength){
	DBG_ENTER("Stack.Length")
	c_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_newlength,"newlength")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<45>");
	if(t_newlength<m_length){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<46>");
		for(int t_i=t_newlength;t_i<m_length;t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<47>");
			gc_assign(m_data.At(t_i),m_NIL);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<49>");
		if(t_newlength>m_data.Length()){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<50>");
			gc_assign(m_data,m_data.Resize(bb_math2_Max(m_length*2+10,t_newlength)));
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<52>");
	m_length=t_newlength;
}
int c_Stack3::p_Length(){
	DBG_ENTER("Stack.Length")
	c_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<56>");
	return m_length;
}
void c_Stack3::mark(){
	Object::mark();
	gc_mark_q(m_data);
}
String c_Stack3::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("NIL",&c_Stack3::m_NIL);
	t+=dbg_decl("data",&m_data);
	t+=dbg_decl("length",&m_length);
	return t;
}
c_Stack4::c_Stack4(){
	m_length=0;
	m_data=Array<c_Point* >();
}
void c_Stack4::p_Push10(c_Point* t_value){
	DBG_ENTER("Stack.Push")
	c_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<71>");
	if(m_length==m_data.Length()){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<72>");
		gc_assign(m_data,m_data.Resize(m_length*2+10));
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<74>");
	gc_assign(m_data.At(m_length),t_value);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<75>");
	m_length+=1;
}
void c_Stack4::p_Push11(Array<c_Point* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	c_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<83>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<84>");
		p_Push10(t_values.At(t_offset+t_i));
	}
}
void c_Stack4::p_Push12(Array<c_Point* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	c_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<79>");
	p_Push11(t_values,t_offset,t_values.Length()-t_offset);
}
c_Point* c_Stack4::m_NIL;
void c_Stack4::p_Length2(int t_newlength){
	DBG_ENTER("Stack.Length")
	c_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_newlength,"newlength")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<45>");
	if(t_newlength<m_length){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<46>");
		for(int t_i=t_newlength;t_i<m_length;t_i=t_i+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i,"i")
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<47>");
			gc_assign(m_data.At(t_i),m_NIL);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<49>");
		if(t_newlength>m_data.Length()){
			DBG_BLOCK();
			DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<50>");
			gc_assign(m_data,m_data.Resize(bb_math2_Max(m_length*2+10,t_newlength)));
		}
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<52>");
	m_length=t_newlength;
}
int c_Stack4::p_Length(){
	DBG_ENTER("Stack.Length")
	c_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<56>");
	return m_length;
}
c_Point* c_Stack4::p_Get(int t_index){
	DBG_ENTER("Stack.Get")
	c_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<104>");
	return m_data.At(t_index);
}
c_Point* c_Stack4::p_Pop(){
	DBG_ENTER("Stack.Pop")
	c_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<89>");
	m_length-=1;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<90>");
	c_Point* t_v=m_data.At(m_length);
	DBG_LOCAL(t_v,"v")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<91>");
	gc_assign(m_data.At(m_length),m_NIL);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<92>");
	return t_v;
}
void c_Stack4::mark(){
	Object::mark();
	gc_mark_q(m_data);
}
String c_Stack4::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("NIL",&c_Stack4::m_NIL);
	t+=dbg_decl("data",&m_data);
	t+=dbg_decl("length",&m_length);
	return t;
}
int bb_math2_Max(int t_x,int t_y){
	DBG_ENTER("Max")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<56>");
	if(t_x>t_y){
		DBG_BLOCK();
		return t_x;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<57>");
	return t_y;
}
Float bb_math2_Max2(Float t_x,Float t_y){
	DBG_ENTER("Max")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<83>");
	if(t_x>t_y){
		DBG_BLOCK();
		return t_x;
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<84>");
	return t_y;
}
Float bb_level_getDist(int t_sX,int t_sY,int t_tX,int t_tY){
	DBG_ENTER("getDist")
	DBG_LOCAL(t_sX,"sX")
	DBG_LOCAL(t_sY,"sY")
	DBG_LOCAL(t_tX,"tX")
	DBG_LOCAL(t_tY,"tY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<687>");
	int t_dx=t_tX-t_sX;
	DBG_LOCAL(t_dx,"dx")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<688>");
	int t_dy=t_tY-t_sY;
	DBG_LOCAL(t_dy,"dy")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<690>");
	Float t_=(Float)sqrt(Float(t_dx*t_dx+t_dy*t_dy));
	return t_;
}
Float bb_pathfinder_getCost(int t_sX,int t_sY,int t_tX,int t_tY){
	DBG_ENTER("getCost")
	DBG_LOCAL(t_sX,"sX")
	DBG_LOCAL(t_sY,"sY")
	DBG_LOCAL(t_tX,"tX")
	DBG_LOCAL(t_tY,"tY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<472>");
	int t_dx=t_tX-t_sX;
	DBG_LOCAL(t_dx,"dx")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<473>");
	int t_dy=t_tY-t_sY;
	DBG_LOCAL(t_dy,"dy")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<475>");
	Float t_=(Float)sqrt(Float(t_dx*t_dx+t_dy*t_dy));
	return t_;
}
bool bb_pathfinder_stackHasElement(c_Stack2* t_s,c_AStarNode* t_n){
	DBG_ENTER("stackHasElement")
	DBG_LOCAL(t_s,"s")
	DBG_LOCAL(t_n,"n")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<481>");
	for(int t_i=0;t_i<t_s->p_Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<482>");
		if(t_s->p_Get(t_i)->m_id==t_n->m_id){
			DBG_BLOCK();
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<484>");
			return true;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/pathfinder.monkey<488>");
	return false;
}
c_BoundingRect::c_BoundingRect(){
	m_x=0;
	m_y=0;
	m_width=0;
	m_height=0;
	m_top=0;
	m_bottom=0;
	m_left=0;
	m_right=0;
}
c_BoundingRect* c_BoundingRect::m_new(int t_x,int t_y,int t_w,int t_h){
	DBG_ENTER("BoundingRect.new")
	c_BoundingRect *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_w,"w")
	DBG_LOCAL(t_h,"h")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<258>");
	this->m_x=t_x;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<259>");
	this->m_y=t_y;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<260>");
	this->m_width=t_w;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<261>");
	this->m_height=t_h;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<263>");
	this->m_top=t_y;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<264>");
	this->m_bottom=t_y+t_h;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<265>");
	this->m_left=t_x;
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<266>");
	this->m_right=t_x+t_w;
	return this;
}
c_BoundingRect* c_BoundingRect::m_new2(){
	DBG_ENTER("BoundingRect.new")
	c_BoundingRect *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/gameplayscene.monkey<246>");
	return this;
}
void c_BoundingRect::mark(){
	Object::mark();
}
String c_BoundingRect::debug(){
	String t="(BoundingRect)\n";
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	t+=dbg_decl("width",&m_width);
	t+=dbg_decl("height",&m_height);
	t+=dbg_decl("top",&m_top);
	t+=dbg_decl("bottom",&m_bottom);
	t+=dbg_decl("left",&m_left);
	t+=dbg_decl("right",&m_right);
	return t;
}
int bb_input_KeyDown(int t_key){
	DBG_ENTER("KeyDown")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<40>");
	int t_=((bb_input_device->p_KeyDown(t_key))?1:0);
	return t_;
}
c_Image* bb_noisetestscene_textures;
c_Image* bb_noisetestscene_enemies;
c_SimplexNoise::c_SimplexNoise(){
	m_grad3=Array<Array<int > >();
	m_p=Array<int >();
	m_perm=Array<int >();
}
c_SimplexNoise* c_SimplexNoise::m_new(){
	DBG_ENTER("SimplexNoise.new")
	c_SimplexNoise *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<18>");
	int t_[]={1,1,0};
	int t_2[]={-1,1,0};
	int t_3[]={1,-1,0};
	int t_4[]={-1,-1,0};
	int t_5[]={1,0,1};
	int t_6[]={-1,0,1};
	int t_7[]={1,0,-1};
	int t_8[]={-1,0,-1};
	int t_9[]={0,1,1};
	int t_10[]={0,-1,1};
	int t_11[]={0,1,-1};
	int t_12[]={0,-1,-1};
	Array<int > t_13[]={Array<int >(t_,3),Array<int >(t_2,3),Array<int >(t_3,3),Array<int >(t_4,3),Array<int >(t_5,3),Array<int >(t_6,3),Array<int >(t_7,3),Array<int >(t_8,3),Array<int >(t_9,3),Array<int >(t_10,3),Array<int >(t_11,3),Array<int >(t_12,3)};
	gc_assign(m_grad3,Array<Array<int > >(t_13,12));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<31>");
	int t_14[]={151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};
	gc_assign(m_p,Array<int >(t_14,256));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<32>");
	gc_assign(m_perm,Array<int >(512));
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<33>");
	for(int t_i=0;t_i<512;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<34>");
		m_perm.At(t_i)=m_p.At(t_i&255);
	}
	return this;
}
Array<Array<Float > > c_SimplexNoise::p_setArray(int t_i,int t_j){
	DBG_ENTER("SimplexNoise.setArray")
	c_SimplexNoise *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_i,"i")
	DBG_LOCAL(t_j,"j")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<235>");
	Array<Array<Float > > t_result=Array<Array<Float > >(t_i);
	DBG_LOCAL(t_result,"result")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<237>");
	for(int t_index=0;t_index<t_i;t_index=t_index+1){
		DBG_BLOCK();
		DBG_LOCAL(t_index,"index")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<238>");
		gc_assign(t_result.At(t_index),Array<Float >(t_j));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<241>");
	return t_result;
}
Float c_SimplexNoise::p_dot(Array<int > t_g,Float t_x,Float t_y){
	DBG_ENTER("SimplexNoise.dot")
	c_SimplexNoise *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_g,"g")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<43>");
	Float t_=Float(t_g.At(0))*t_x+Float(t_g.At(1))*t_y;
	return t_;
}
Float c_SimplexNoise::p_makeNoise(Float t_x,Float t_y){
	DBG_ENTER("SimplexNoise.makeNoise")
	c_SimplexNoise *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<52>");
	Float t_n0=FLOAT(.0);
	DBG_LOCAL(t_n0,"n0")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<53>");
	Float t_n1=FLOAT(.0);
	DBG_LOCAL(t_n1,"n1")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<54>");
	Float t_n2=FLOAT(.0);
	DBG_LOCAL(t_n2,"n2")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<56>");
	Float t_f2=FLOAT(0.5)*((Float)sqrt(FLOAT(3.0))-FLOAT(1.0));
	DBG_LOCAL(t_f2,"f2")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<57>");
	Float t_s=(t_x+t_y)*t_f2;
	DBG_LOCAL(t_s,"s")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<58>");
	int t_i=int((Float)floor(t_x+t_s));
	DBG_LOCAL(t_i,"i")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<59>");
	int t_j=int((Float)floor(t_y+t_s));
	DBG_LOCAL(t_j,"j")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<61>");
	Float t_g2=(FLOAT(3.0)-(Float)sqrt(FLOAT(3.0)))/FLOAT(6.0);
	DBG_LOCAL(t_g2,"g2")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<62>");
	Float t_t=Float(t_i+t_j)*t_g2;
	DBG_LOCAL(t_t,"t")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<63>");
	Float t_x0=Float(t_i)-t_t;
	DBG_LOCAL(t_x0,"x0")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<64>");
	Float t_y0=Float(t_j)-t_t;
	DBG_LOCAL(t_y0,"y0")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<65>");
	Float t_x01=t_x-t_x0;
	DBG_LOCAL(t_x01,"x01")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<66>");
	Float t_y01=t_y-t_y0;
	DBG_LOCAL(t_y01,"y01")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<69>");
	int t_i1=0;
	DBG_LOCAL(t_i1,"i1")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<70>");
	int t_j1=0;
	DBG_LOCAL(t_j1,"j1")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<71>");
	if(t_x01>t_y01){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<72>");
		t_i1=1;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<73>");
		t_j1=0;
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<75>");
		t_i1=0;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<76>");
		t_j1=1;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<79>");
	Float t_x1=t_x01-Float(t_i1)+t_g2;
	DBG_LOCAL(t_x1,"x1")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<80>");
	Float t_y1=t_y01-Float(t_j1)+t_g2;
	DBG_LOCAL(t_y1,"y1")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<81>");
	Float t_x2=t_x01-FLOAT(1.0)+FLOAT(2.0)*t_g2;
	DBG_LOCAL(t_x2,"x2")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<82>");
	Float t_y2=t_y01-FLOAT(1.0)+FLOAT(2.0)*t_g2;
	DBG_LOCAL(t_y2,"y2")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<85>");
	int t_ii=t_i&255;
	DBG_LOCAL(t_ii,"ii")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<86>");
	int t_jj=t_j&255;
	DBG_LOCAL(t_jj,"jj")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<88>");
	int t_gi0=m_perm.At(t_ii+m_perm.At(t_jj)) % 12;
	DBG_LOCAL(t_gi0,"gi0")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<89>");
	int t_gi1=m_perm.At(t_ii+t_i1+m_perm.At(t_jj+t_j1)) % 12;
	DBG_LOCAL(t_gi1,"gi1")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<90>");
	int t_gi2=m_perm.At(t_ii+1+m_perm.At(t_jj+1)) % 12;
	DBG_LOCAL(t_gi2,"gi2")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<93>");
	Float t_t0=FLOAT(0.5)-t_x01*t_x01-t_y01*t_y01;
	DBG_LOCAL(t_t0,"t0")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<94>");
	if(t_t0<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<95>");
		t_n0=FLOAT(0.0);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<97>");
		t_t0*=t_t0;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<98>");
		t_n0=t_t0*t_t0*p_dot(m_grad3.At(t_gi0),t_x01,t_y01);
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<101>");
	Float t_t1=FLOAT(0.5)-t_x1*t_x1-t_y1*t_y1;
	DBG_LOCAL(t_t1,"t1")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<102>");
	if(t_t1<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<103>");
		t_n1=FLOAT(0.0);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<105>");
		t_t1*=t_t1;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<106>");
		t_n1=t_t1*t_t1*p_dot(m_grad3.At(t_gi1),t_x1,t_y1);
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<109>");
	Float t_t2=FLOAT(0.5)-t_x2*t_x2-t_y2*t_y2;
	DBG_LOCAL(t_t2,"t2")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<110>");
	if(t_t2<FLOAT(0.0)){
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<111>");
		t_n2=FLOAT(0.0);
	}else{
		DBG_BLOCK();
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<113>");
		t_t2*=t_t2;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<114>");
		t_n2=t_t2*t_t2*p_dot(m_grad3.At(t_gi2),t_x2,t_y2);
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<119>");
	Float t_=FLOAT(70.0)*(t_n0+t_n1+t_n2);
	return t_;
}
Float c_SimplexNoise::p_makeOctavedNoise(int t_octaves,Float t_roughness,Float t_scale,int t_x,int t_y,int t_offset){
	DBG_ENTER("SimplexNoise.makeOctavedNoise")
	c_SimplexNoise *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_octaves,"octaves")
	DBG_LOCAL(t_roughness,"roughness")
	DBG_LOCAL(t_scale,"scale")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<215>");
	Float t_noiseSum=FLOAT(0.0);
	DBG_LOCAL(t_noiseSum,"noiseSum")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<216>");
	Float t_layerFrequency=t_scale;
	DBG_LOCAL(t_layerFrequency,"layerFrequency")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<217>");
	Float t_layerWeight=FLOAT(1.0);
	DBG_LOCAL(t_layerWeight,"layerWeight")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<218>");
	Float t_weightSum=FLOAT(0.0);
	DBG_LOCAL(t_weightSum,"weightSum")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<222>");
	for(int t_i=0;t_i<t_octaves;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<223>");
		t_noiseSum+=p_makeNoise(Float(t_x+t_offset)*t_layerFrequency,Float(t_y+t_offset)*t_layerFrequency)*t_layerWeight;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<224>");
		t_layerFrequency=t_layerFrequency*FLOAT(2.0);
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<225>");
		t_weightSum+=t_layerWeight;
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<226>");
		t_layerWeight*=t_roughness;
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<228>");
	Float t_=t_noiseSum/t_weightSum;
	return t_;
}
Array<Array<Float > > c_SimplexNoise::p_makeIsland(Array<Array<Float > > t_noiseMap,int t_width,int t_height){
	DBG_ENTER("SimplexNoise.makeIsland")
	c_SimplexNoise *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_noiseMap,"noiseMap")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<186>");
	Array<Array<Float > > t_island=p_setArray(t_width,t_height);
	DBG_LOCAL(t_island,"island")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<188>");
	int t_centerX=t_width/2;
	DBG_LOCAL(t_centerX,"centerX")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<189>");
	int t_centerY=t_height/2;
	DBG_LOCAL(t_centerY,"centerY")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<194>");
	Float t_xDist=FLOAT(0.0);
	DBG_LOCAL(t_xDist,"xDist")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<195>");
	Float t_yDist=FLOAT(0.0);
	DBG_LOCAL(t_yDist,"yDist")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<196>");
	Float t_totalDist=FLOAT(0.0);
	DBG_LOCAL(t_totalDist,"totalDist")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<198>");
	for(int t_i=0;t_i<t_width;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<199>");
		for(int t_j=0;t_j<t_height;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<200>");
			t_xDist=Float((t_centerX-t_i)*(t_centerX-t_i));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<201>");
			t_yDist=Float((t_centerY-t_j)*(t_centerY-t_j));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<203>");
			t_totalDist=(Float)sqrt(t_xDist+t_yDist)/Float(t_width);
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<204>");
			t_noiseMap.At(t_i).At(t_j)-=t_totalDist;
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<207>");
	return t_noiseMap;
}
Array<Array<Float > > c_SimplexNoise::p_generateOctavedNoiseMap(int t_width,int t_height,int t_octaves,Float t_roughness,Float t_scale){
	DBG_ENTER("SimplexNoise.generateOctavedNoiseMap")
	c_SimplexNoise *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_LOCAL(t_octaves,"octaves")
	DBG_LOCAL(t_roughness,"roughness")
	DBG_LOCAL(t_scale,"scale")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<151>");
	Array<Array<Float > > t_result=p_setArray(t_width,t_height);
	DBG_LOCAL(t_result,"result")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<152>");
	bb_random_Seed=bb_app_Millisecs();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<153>");
	int t_offset=int(bb_random_Rnd2(FLOAT(0.0),FLOAT(1000000.0)));
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<155>");
	Float t_layerFrequency=t_scale;
	DBG_LOCAL(t_layerFrequency,"layerFrequency")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<156>");
	Float t_layerWeight=FLOAT(1.0);
	DBG_LOCAL(t_layerWeight,"layerWeight")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<157>");
	Float t_weightSum=FLOAT(0.0);
	DBG_LOCAL(t_weightSum,"weightSum")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<161>");
	for(int t_i=0;t_i<t_width;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<162>");
		for(int t_j=0;t_j<t_height;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<163>");
			t_result.At(t_i).At(t_j)=p_makeOctavedNoise(5,FLOAT(0.5),FLOAT(0.01),t_i,t_j,t_offset);
		}
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<175>");
	t_result=p_makeIsland(t_result,t_width,t_height);
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<179>");
	return t_result;
}
void c_SimplexNoise::mark(){
	Object::mark();
	gc_mark_q(m_grad3);
	gc_mark_q(m_p);
	gc_mark_q(m_perm);
}
String c_SimplexNoise::debug(){
	String t="(SimplexNoise)\n";
	t+=dbg_decl("grad3",&m_grad3);
	t+=dbg_decl("p",&m_p);
	t+=dbg_decl("perm",&m_perm);
	return t;
}
Array<Array<int > > bb_noisetestscene_setArray(int t_i,int t_j){
	DBG_ENTER("setArray")
	DBG_LOCAL(t_i,"i")
	DBG_LOCAL(t_j,"j")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<910>");
	Array<Array<int > > t_result=Array<Array<int > >(t_i);
	DBG_LOCAL(t_result,"result")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<912>");
	for(int t_index=0;t_index<t_i;t_index=t_index+1){
		DBG_BLOCK();
		DBG_LOCAL(t_index,"index")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<913>");
		gc_assign(t_result.At(t_index),Array<int >(t_j));
	}
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<916>");
	return t_result;
}
int bb_noisetestscene_randomlyAssignCells(Array<Array<int > > t_cells,int t_threshold){
	DBG_ENTER("randomlyAssignCells")
	DBG_LOCAL(t_cells,"cells")
	DBG_LOCAL(t_threshold,"threshold")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<889>");
	bb_random_Seed=bb_app_Millisecs();
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<890>");
	int t_rand=0;
	DBG_LOCAL(t_rand,"rand")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<891>");
	int t_thresh=t_threshold % 100;
	DBG_LOCAL(t_thresh,"thresh")
	DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<893>");
	for(int t_i=0;t_i<t_cells.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<894>");
		for(int t_j=0;t_j<t_cells.At(0).Length();t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<895>");
			t_rand=int(bb_random_Rnd2(FLOAT(0.0),FLOAT(100.0)));
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<896>");
			if(t_rand<t_thresh){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<897>");
				t_cells.At(t_i).At(t_j)=1;
			}else{
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<899>");
				t_cells.At(t_i).At(t_j)=-1;
			}
			DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<901>");
			if(t_i==0 || t_j==0 || t_i==t_cells.Length()-1 || t_j==t_cells.At(0).Length()-1 || t_cells.At(t_i).At(t_j)==9){
				DBG_BLOCK();
				DBG_INFO("/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<902>");
				t_cells.At(t_i).At(t_j)=-1;
			}
		}
	}
	return 0;
}
c_Enumerator::c_Enumerator(){
	m__list=0;
	m__curr=0;
}
c_Enumerator* c_Enumerator::m_new(c_List* t_list){
	DBG_ENTER("Enumerator.new")
	c_Enumerator *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_list,"list")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<326>");
	gc_assign(m__list,t_list);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<327>");
	gc_assign(m__curr,t_list->m__head->m__succ);
	return this;
}
c_Enumerator* c_Enumerator::m_new2(){
	DBG_ENTER("Enumerator.new")
	c_Enumerator *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<323>");
	return this;
}
bool c_Enumerator::p_HasNext(){
	DBG_ENTER("Enumerator.HasNext")
	c_Enumerator *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<331>");
	while(m__curr->m__succ->m__pred!=m__curr){
		DBG_BLOCK();
		DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<332>");
		gc_assign(m__curr,m__curr->m__succ);
	}
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<334>");
	bool t_=m__curr!=m__list->m__head;
	return t_;
}
c_PathStep* c_Enumerator::p_NextObject(){
	DBG_ENTER("Enumerator.NextObject")
	c_Enumerator *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<338>");
	c_PathStep* t_data=m__curr->m__data;
	DBG_LOCAL(t_data,"data")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<339>");
	gc_assign(m__curr,m__curr->m__succ);
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/list.monkey<340>");
	return t_data;
}
void c_Enumerator::mark(){
	Object::mark();
	gc_mark_q(m__list);
	gc_mark_q(m__curr);
}
String c_Enumerator::debug(){
	String t="(Enumerator)\n";
	t+=dbg_decl("_list",&m__list);
	t+=dbg_decl("_curr",&m__curr);
	return t;
}
c_iConfig::c_iConfig(){
}
void c_iConfig::mark(){
	Object::mark();
}
String c_iConfig::debug(){
	String t="(iConfig)\n";
	return t;
}
c_iVector2d::c_iVector2d(){
	m__x=FLOAT(0.0);
	m__y=FLOAT(0.0);
}
c_iVector2d* c_iVector2d::m_new(Float t_x,Float t_y){
	DBG_ENTER("iVector2d.new")
	c_iVector2d *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/vector2d.monkey<31>");
	m__x=t_x;
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/vector2d.monkey<32>");
	m__y=t_y;
	return this;
}
c_iVector2d* c_iVector2d::m_new2(){
	DBG_ENTER("iVector2d.new")
	c_iVector2d *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/vector2d.monkey<15>");
	return this;
}
void c_iVector2d::mark(){
	Object::mark();
}
String c_iVector2d::debug(){
	String t="(iVector2d)\n";
	t+=dbg_decl("_x",&m__x);
	t+=dbg_decl("_y",&m__y);
	return t;
}
c_iControlSet::c_iControlSet(){
}
void c_iControlSet::mark(){
	Object::mark();
}
String c_iControlSet::debug(){
	String t="(iControlSet)\n";
	return t;
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
	bb_app2_iCurrentScene=0;
	DBG_GLOBAL("iCurrentScene",&bb_app2_iCurrentScene);
	bb_app2_iDT=(new c_iDeltaTimer)->m_new(FLOAT(60.0));
	DBG_GLOBAL("iDT",&bb_app2_iDT);
	bb_app2_iSpeed=1;
	DBG_GLOBAL("iSpeed",&bb_app2_iSpeed);
	bb_app2_iNextScene=0;
	DBG_GLOBAL("iNextScene",&bb_app2_iNextScene);
	bb_random_Seed=1234;
	DBG_GLOBAL("Seed",&bb_random_Seed);
	bb_main_menu=0;
	DBG_GLOBAL("menu",&bb_main_menu);
	bb_main_gameplay=0;
	DBG_GLOBAL("gameplay",&bb_main_gameplay);
	bb_main_noiseTest=0;
	DBG_GLOBAL("noiseTest",&bb_main_noiseTest);
	bb_app__updateRate=0;
	DBG_GLOBAL("_updateRate",&bb_app__updateRate);
	bb_globals_iEnginePointer=0;
	DBG_GLOBAL("iEnginePointer",&bb_globals_iEnginePointer);
	c_iStack2D::m__length=0;
	DBG_GLOBAL("_length",&c_iStack2D::m__length);
	c_iStack2D::m__data=Array<c_iStack2D* >();
	DBG_GLOBAL("_data",&c_iStack2D::m__data);
	bb_globals_iRotation=-1;
	DBG_GLOBAL("iRotation",&bb_globals_iRotation);
	c_iSystemGui::m__playfield=0;
	DBG_GLOBAL("_playfield",&c_iSystemGui::m__playfield);
	bb_globals_iTaskList=(new c_iList5)->m_new();
	DBG_GLOBAL("iTaskList",&bb_globals_iTaskList);
	c_iGuiObject::m__topObject=0;
	DBG_GLOBAL("_topObject",&c_iGuiObject::m__topObject);
	bb_globals_iMultiTouch=8;
	DBG_GLOBAL("iMultiTouch",&bb_globals_iMultiTouch);
	c_iEngineObject::m_destroyList=(new c_iList6)->m_new();
	DBG_GLOBAL("destroyList",&c_iEngineObject::m_destroyList);
	bb_contentmanager_iContent=(new c_iContentManager)->m_new();
	DBG_GLOBAL("iContent",&bb_contentmanager_iContent);
	bb_level_caveTextures=0;
	DBG_GLOBAL("caveTextures",&bb_level_caveTextures);
	c_Stack4::m_NIL=0;
	DBG_GLOBAL("NIL",&c_Stack4::m_NIL);
	c_Stack3::m_NIL=0;
	DBG_GLOBAL("NIL",&c_Stack3::m_NIL);
	c_Stack2::m_NIL=0;
	DBG_GLOBAL("NIL",&c_Stack2::m_NIL);
	bb_noisetestscene_textures=0;
	DBG_GLOBAL("textures",&bb_noisetestscene_textures);
	bb_noisetestscene_enemies=0;
	DBG_GLOBAL("enemies",&bb_noisetestscene_enemies);
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
	gc_mark_q(bb_app2_iCurrentScene);
	gc_mark_q(bb_app2_iDT);
	gc_mark_q(bb_app2_iNextScene);
	gc_mark_q(bb_main_menu);
	gc_mark_q(bb_main_gameplay);
	gc_mark_q(bb_main_noiseTest);
	gc_mark_q(bb_globals_iEnginePointer);
	gc_mark_q(c_iStack2D::m__data);
	gc_mark_q(c_iSystemGui::m__playfield);
	gc_mark_q(bb_globals_iTaskList);
	gc_mark_q(c_iGuiObject::m__topObject);
	gc_mark_q(c_iEngineObject::m_destroyList);
	gc_mark_q(bb_contentmanager_iContent);
	gc_mark_q(bb_level_caveTextures);
	gc_mark_q(c_Stack4::m_NIL);
	gc_mark_q(c_Stack3::m_NIL);
	gc_mark_q(c_Stack2::m_NIL);
	gc_mark_q(bb_noisetestscene_textures);
	gc_mark_q(bb_noisetestscene_enemies);
}
//${TRANSCODE_END}

int main( int argc,const char *argv[] ){

	BBMonkeyGame::Main( argc,argv );
}
