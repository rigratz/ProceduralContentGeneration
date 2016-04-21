
//${CONFIG_BEGIN}
CFG_BINARY_FILES="*.bin|*.dat";
CFG_BRL_GAMETARGET_IMPLEMENTED="1";
CFG_BRL_THREAD_IMPLEMENTED="1";
CFG_CONFIG="debug";
CFG_HOST="macos";
CFG_HTML5_WEBAUDIO_ENABLED="1";
CFG_IGNITION_ALLOW_SET_ALPHA_FOR_EACH_OBJECT="1";
CFG_IGNITION_ALLOW_SET_BLEND_FOR_EACH_OBJECT="1";
CFG_IGNITION_ALLOW_SET_COLOR_FOR_EACH_OBJECT="1";
CFG_IGNITION_EDITOR_TOOLS="0";
CFG_IMAGE_FILES="*.png|*.jpg";
CFG_LANG="js";
CFG_MOJO_AUTO_SUSPEND_ENABLED="1";
CFG_MOJO_DRIVER_IMPLEMENTED="1";
CFG_MUSIC_FILES="*.wav|*.ogg|*.mp3|*.m4a";
CFG_OPENGL_GLES20_ENABLED="0";
CFG_SAFEMODE="0";
CFG_SOUND_FILES="*.wav|*.ogg|*.mp3|*.m4a";
CFG_TARGET="html5";
CFG_TEXT_FILES="*.txt|*.xml|*.json";
//${CONFIG_END}

//${METADATA_BEGIN}
var META_DATA="[char_walk_down.png];type=image/png;width=276;height=102;\n[chocobo.png];type=image/png;width=60;height=64;\n[rockstar.png];type=image/png;width=69;height=102;\n[mojo_font.png];type=image/png;width=864;height=13;\n";
//${METADATA_END}

//${TRANSCODE_BEGIN}

// Javascript Monkey runtime.
//
// Placed into the public domain 24/02/2011.
// No warranty implied; use at your own risk.

//***** JavaScript Runtime *****

var D2R=0.017453292519943295;
var R2D=57.29577951308232;

var err_info="";
var err_stack=[];

var dbg_index=0;

function push_err(){
	err_stack.push( err_info );
}

function pop_err(){
	err_info=err_stack.pop();
}

function stackTrace(){
	if( !err_info.length ) return "";
	var str=err_info+"\n";
	for( var i=err_stack.length-1;i>0;--i ){
		str+=err_stack[i]+"\n";
	}
	return str;
}

function print( str ){
	var cons=document.getElementById( "GameConsole" );
	if( cons ){
		cons.value+=str+"\n";
		cons.scrollTop=cons.scrollHeight-cons.clientHeight;
	}else if( window.console!=undefined ){
		window.console.log( str );
	}
	return 0;
}

function alertError( err ){
	if( typeof(err)=="string" && err=="" ) return;
	alert( "Monkey Runtime Error : "+err.toString()+"\n\n"+stackTrace() );
}

function error( err ){
	throw err;
}

function debugLog( str ){
	if( window.console!=undefined ) window.console.log( str );
}

function debugStop(){
	debugger;	//	error( "STOP" );
}

function dbg_object( obj ){
	if( obj ) return obj;
	error( "Null object access" );
}

function dbg_charCodeAt( str,index ){
	if( index<0 || index>=str.length ) error( "Character index out of range" );
	return str.charCodeAt( index );
}

function dbg_array( arr,index ){
	if( index<0 || index>=arr.length ) error( "Array index out of range" );
	dbg_index=index;
	return arr;
}

function new_bool_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=false;
	return arr;
}

function new_number_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=0;
	return arr;
}

function new_string_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]='';
	return arr;
}

function new_array_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=[];
	return arr;
}

function new_object_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=null;
	return arr;
}

function resize_bool_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=false;
	return arr;
}

function resize_number_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=0;
	return arr;
}

function resize_string_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]="";
	return arr;
}

function resize_array_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=[];
	return arr;
}

function resize_object_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=null;
	return arr;
}

function string_compare( lhs,rhs ){
	var n=Math.min( lhs.length,rhs.length ),i,t;
	for( i=0;i<n;++i ){
		t=lhs.charCodeAt(i)-rhs.charCodeAt(i);
		if( t ) return t;
	}
	return lhs.length-rhs.length;
}

function string_replace( str,find,rep ){	//no unregex replace all?!?
	var i=0;
	for(;;){
		i=str.indexOf( find,i );
		if( i==-1 ) return str;
		str=str.substring( 0,i )+rep+str.substring( i+find.length );
		i+=rep.length;
	}
}

function string_trim( str ){
	var i=0,i2=str.length;
	while( i<i2 && str.charCodeAt(i)<=32 ) i+=1;
	while( i2>i && str.charCodeAt(i2-1)<=32 ) i2-=1;
	return str.slice( i,i2 );
}

function string_startswith( str,substr ){
	return substr.length<=str.length && str.slice(0,substr.length)==substr;
}

function string_endswith( str,substr ){
	return substr.length<=str.length && str.slice(str.length-substr.length,str.length)==substr;
}

function string_tochars( str ){
	var arr=new Array( str.length );
	for( var i=0;i<str.length;++i ) arr[i]=str.charCodeAt(i);
	return arr;
}

function string_fromchars( chars ){
	var str="",i;
	for( i=0;i<chars.length;++i ){
		str+=String.fromCharCode( chars[i] );
	}
	return str;
}

function object_downcast( obj,clas ){
	if( obj instanceof clas ) return obj;
	return null;
}

function object_implements( obj,iface ){
	if( obj && obj.implments && obj.implments[iface] ) return obj;
	return null;
}

function extend_class( clas ){
	var tmp=function(){};
	tmp.prototype=clas.prototype;
	return new tmp;
}

function ThrowableObject(){
}

ThrowableObject.prototype.toString=function(){ 
	return "Uncaught Monkey Exception"; 
}


function BBGameEvent(){}
BBGameEvent.KeyDown=1;
BBGameEvent.KeyUp=2;
BBGameEvent.KeyChar=3;
BBGameEvent.MouseDown=4;
BBGameEvent.MouseUp=5;
BBGameEvent.MouseMove=6;
BBGameEvent.TouchDown=7;
BBGameEvent.TouchUp=8;
BBGameEvent.TouchMove=9;
BBGameEvent.MotionAccel=10;

function BBGameDelegate(){}
BBGameDelegate.prototype.StartGame=function(){}
BBGameDelegate.prototype.SuspendGame=function(){}
BBGameDelegate.prototype.ResumeGame=function(){}
BBGameDelegate.prototype.UpdateGame=function(){}
BBGameDelegate.prototype.RenderGame=function(){}
BBGameDelegate.prototype.KeyEvent=function( ev,data ){}
BBGameDelegate.prototype.MouseEvent=function( ev,data,x,y ){}
BBGameDelegate.prototype.TouchEvent=function( ev,data,x,y ){}
BBGameDelegate.prototype.MotionEvent=function( ev,data,x,y,z ){}
BBGameDelegate.prototype.DiscardGraphics=function(){}

function BBDisplayMode( width,height ){
	this.width=width;
	this.height=height;
}

function BBGame(){
	BBGame._game=this;
	this._delegate=null;
	this._keyboardEnabled=false;
	this._updateRate=0;
	this._started=false;
	this._suspended=false;
	this._debugExs=(CFG_CONFIG=="debug");
	this._startms=Date.now();
}

BBGame.Game=function(){
	return BBGame._game;
}

BBGame.prototype.SetDelegate=function( delegate ){
	this._delegate=delegate;
}

BBGame.prototype.Delegate=function(){
	return this._delegate;
}

BBGame.prototype.SetUpdateRate=function( updateRate ){
	this._updateRate=updateRate;
}

BBGame.prototype.SetKeyboardEnabled=function( keyboardEnabled ){
	this._keyboardEnabled=keyboardEnabled;
}

BBGame.prototype.Started=function(){
	return this._started;
}

BBGame.prototype.Suspended=function(){
	return this._suspended;
}

BBGame.prototype.Millisecs=function(){
	return Date.now()-this._startms;
}

BBGame.prototype.GetDate=function( date ){
	var n=date.length;
	if( n>0 ){
		var t=new Date();
		date[0]=t.getFullYear();
		if( n>1 ){
			date[1]=t.getMonth()+1;
			if( n>2 ){
				date[2]=t.getDate();
				if( n>3 ){
					date[3]=t.getHours();
					if( n>4 ){
						date[4]=t.getMinutes();
						if( n>5 ){
							date[5]=t.getSeconds();
							if( n>6 ){
								date[6]=t.getMilliseconds();
							}
						}
					}
				}
			}
		}
	}
}

BBGame.prototype.SaveState=function( state ){
	localStorage.setItem( "monkeystate@"+document.URL,state );	//key can't start with dot in Chrome!
	return 1;
}

BBGame.prototype.LoadState=function(){
	var state=localStorage.getItem( "monkeystate@"+document.URL );
	if( state ) return state;
	return "";
}

BBGame.prototype.LoadString=function( path ){

	var xhr=new XMLHttpRequest();
	xhr.open( "GET",this.PathToUrl( path ),false );
	
	xhr.send( null );
	
	if( xhr.status==200 || xhr.status==0 ) return xhr.responseText;
	
	return "";
}

BBGame.prototype.PollJoystick=function( port,joyx,joyy,joyz,buttons ){
	return false;
}

BBGame.prototype.OpenUrl=function( url ){
	window.location=url;
}

BBGame.prototype.SetMouseVisible=function( visible ){
	if( visible ){
		this._canvas.style.cursor='default';	
	}else{
		this._canvas.style.cursor="url('data:image/cur;base64,AAACAAEAICAAAAAAAACoEAAAFgAAACgAAAAgAAAAQAAAAAEAIAAAAAAAgBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA55ZXBgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOeWVxAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADnllcGAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////9////////////////////+////////f/////////8%3D'), auto";
	}
}

BBGame.prototype.GetDeviceWidth=function(){
	return 0;
}

BBGame.prototype.GetDeviceHeight=function(){
	return 0;
}

BBGame.prototype.SetDeviceWindow=function( width,height,flags ){
}

BBGame.prototype.GetDisplayModes=function(){
	return new Array();
}

BBGame.prototype.GetDesktopMode=function(){
	return null;
}

BBGame.prototype.SetSwapInterval=function( interval ){
}

BBGame.prototype.PathToFilePath=function( path ){
	return "";
}

//***** js Game *****

BBGame.prototype.PathToUrl=function( path ){
	return path;
}

BBGame.prototype.LoadData=function( path ){

	var xhr=new XMLHttpRequest();
	xhr.open( "GET",this.PathToUrl( path ),false );

	if( xhr.overrideMimeType ) xhr.overrideMimeType( "text/plain; charset=x-user-defined" );

	xhr.send( null );
	if( xhr.status!=200 && xhr.status!=0 ) return null;

	var r=xhr.responseText;
	var buf=new ArrayBuffer( r.length );
	var bytes=new Int8Array( buf );
	for( var i=0;i<r.length;++i ){
		bytes[i]=r.charCodeAt( i );
	}
	return buf;
}

//***** INTERNAL ******

BBGame.prototype.Die=function( ex ){

	this._delegate=new BBGameDelegate();
	
	if( !ex.toString() ){
		return;
	}
	
	if( this._debugExs ){
		print( "Monkey Runtime Error : "+ex.toString() );
		print( stackTrace() );
	}
	
	throw ex;
}

BBGame.prototype.StartGame=function(){

	if( this._started ) return;
	this._started=true;
	
	if( this._debugExs ){
		try{
			this._delegate.StartGame();
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.StartGame();
	}
}

BBGame.prototype.SuspendGame=function(){

	if( !this._started || this._suspended ) return;
	this._suspended=true;
	
	if( this._debugExs ){
		try{
			this._delegate.SuspendGame();
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.SuspendGame();
	}
}

BBGame.prototype.ResumeGame=function(){

	if( !this._started || !this._suspended ) return;
	this._suspended=false;
	
	if( this._debugExs ){
		try{
			this._delegate.ResumeGame();
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.ResumeGame();
	}
}

BBGame.prototype.UpdateGame=function(){

	if( !this._started || this._suspended ) return;

	if( this._debugExs ){
		try{
			this._delegate.UpdateGame();
		}catch( ex ){
			this.Die( ex );
		}	
	}else{
		this._delegate.UpdateGame();
	}
}

BBGame.prototype.RenderGame=function(){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.RenderGame();
		}catch( ex ){
			this.Die( ex );
		}	
	}else{
		this._delegate.RenderGame();
	}
}

BBGame.prototype.KeyEvent=function( ev,data ){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.KeyEvent( ev,data );
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.KeyEvent( ev,data );
	}
}

BBGame.prototype.MouseEvent=function( ev,data,x,y ){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.MouseEvent( ev,data,x,y );
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.MouseEvent( ev,data,x,y );
	}
}

BBGame.prototype.TouchEvent=function( ev,data,x,y ){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.TouchEvent( ev,data,x,y );
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.TouchEvent( ev,data,x,y );
	}
}

BBGame.prototype.MotionEvent=function( ev,data,x,y,z ){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.MotionEvent( ev,data,x,y,z );
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.MotionEvent( ev,data,x,y,z );
	}
}

BBGame.prototype.DiscardGraphics=function(){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.DiscardGraphics();
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.DiscardGraphics();
	}
}


function BBHtml5Game( canvas ){
	BBGame.call( this );
	BBHtml5Game._game=this;
	this._canvas=canvas;
	this._loading=0;
	this._timerSeq=0;
	this._gl=null;
	if( CFG_OPENGL_GLES20_ENABLED=="1" ){
		this._gl=this._canvas.getContext( "webgl" );
		if( !this._gl ) this._gl=this._canvas.getContext( "experimental-webgl" );
		if( !this._gl ) this.Die( "Can't create WebGL" );
		gl=this._gl;
	}
}

BBHtml5Game.prototype=extend_class( BBGame );

BBHtml5Game.Html5Game=function(){
	return BBHtml5Game._game;
}

BBHtml5Game.prototype.ValidateUpdateTimer=function(){

	++this._timerSeq;
	if( this._suspended ) return;
	
	var game=this;
	var seq=game._timerSeq;
	
	var maxUpdates=4;
	var updateRate=this._updateRate;
	
	if( !updateRate ){

		var reqAnimFrame=(window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.oRequestAnimationFrame || window.msRequestAnimationFrame);
	
		if( reqAnimFrame ){
			function animate(){
				if( seq!=game._timerSeq ) return;
	
				game.UpdateGame();
				if( seq!=game._timerSeq ) return;
	
				reqAnimFrame( animate );
				game.RenderGame();
			}
			reqAnimFrame( animate );
			return;
		}
		
		maxUpdates=1;
		updateRate=60;
	}
	
	var updatePeriod=1000.0/updateRate;
	var nextUpdate=0;

	function timeElapsed(){
		if( seq!=game._timerSeq ) return;
		
		if( !nextUpdate ) nextUpdate=Date.now();
		
		for( var i=0;i<maxUpdates;++i ){
		
			game.UpdateGame();
			if( seq!=game._timerSeq ) return;
			
			nextUpdate+=updatePeriod;
			var delay=nextUpdate-Date.now();
			
			if( delay>0 ){
				setTimeout( timeElapsed,delay );
				game.RenderGame();
				return;
			}
		}
		nextUpdate=0;
		setTimeout( timeElapsed,0 );
		game.RenderGame();
	}

	setTimeout( timeElapsed,0 );
}

//***** BBGame methods *****

BBHtml5Game.prototype.SetUpdateRate=function( updateRate ){

	BBGame.prototype.SetUpdateRate.call( this,updateRate );
	
	this.ValidateUpdateTimer();
}

BBHtml5Game.prototype.GetMetaData=function( path,key ){
	if( path.indexOf( "monkey://data/" )!=0 ) return "";
	path=path.slice(14);

	var i=META_DATA.indexOf( "["+path+"]" );
	if( i==-1 ) return "";
	i+=path.length+2;

	var e=META_DATA.indexOf( "\n",i );
	if( e==-1 ) e=META_DATA.length;

	i=META_DATA.indexOf( ";"+key+"=",i )
	if( i==-1 || i>=e ) return "";
	i+=key.length+2;

	e=META_DATA.indexOf( ";",i );
	if( e==-1 ) return "";

	return META_DATA.slice( i,e );
}

BBHtml5Game.prototype.PathToUrl=function( path ){
	if( path.indexOf( "monkey:" )!=0 ){
		return path;
	}else if( path.indexOf( "monkey://data/" )==0 ) {
		return "data/"+path.slice( 14 );
	}
	return "";
}

BBHtml5Game.prototype.GetLoading=function(){
	return this._loading;
}

BBHtml5Game.prototype.IncLoading=function(){
	++this._loading;
	return this._loading;
}

BBHtml5Game.prototype.DecLoading=function(){
	--this._loading;
	return this._loading;
}

BBHtml5Game.prototype.GetCanvas=function(){
	return this._canvas;
}

BBHtml5Game.prototype.GetWebGL=function(){
	return this._gl;
}

BBHtml5Game.prototype.GetDeviceWidth=function(){
	return this._canvas.width;
}

BBHtml5Game.prototype.GetDeviceHeight=function(){
	return this._canvas.height;
}

//***** INTERNAL *****

BBHtml5Game.prototype.UpdateGame=function(){

	if( !this._loading ) BBGame.prototype.UpdateGame.call( this );
}

BBHtml5Game.prototype.SuspendGame=function(){

	BBGame.prototype.SuspendGame.call( this );
	
	BBGame.prototype.RenderGame.call( this );
	
	this.ValidateUpdateTimer();
}

BBHtml5Game.prototype.ResumeGame=function(){

	BBGame.prototype.ResumeGame.call( this );
	
	this.ValidateUpdateTimer();
}

BBHtml5Game.prototype.Run=function(){

	var game=this;
	var canvas=game._canvas;
	
	var xscale=1;
	var yscale=1;
	
	var touchIds=new Array( 32 );
	for( i=0;i<32;++i ) touchIds[i]=-1;
	
	function eatEvent( e ){
		if( e.stopPropagation ){
			e.stopPropagation();
			e.preventDefault();
		}else{
			e.cancelBubble=true;
			e.returnValue=false;
		}
	}
	
	function keyToChar( key ){
		switch( key ){
		case 8:case 9:case 13:case 27:case 32:return key;
		case 33:case 34:case 35:case 36:case 37:case 38:case 39:case 40:case 45:return key|0x10000;
		case 46:return 127;
		}
		return 0;
	}
	
	function mouseX( e ){
		var x=e.clientX+document.body.scrollLeft;
		var c=canvas;
		while( c ){
			x-=c.offsetLeft;
			c=c.offsetParent;
		}
		return x*xscale;
	}
	
	function mouseY( e ){
		var y=e.clientY+document.body.scrollTop;
		var c=canvas;
		while( c ){
			y-=c.offsetTop;
			c=c.offsetParent;
		}
		return y*yscale;
	}

	function touchX( touch ){
		var x=touch.pageX;
		var c=canvas;
		while( c ){
			x-=c.offsetLeft;
			c=c.offsetParent;
		}
		return x;
	}			
	
	function touchY( touch ){
		var y=touch.pageY;
		var c=canvas;
		while( c ){
			y-=c.offsetTop;
			c=c.offsetParent;
		}
		return y;
	}
	
	canvas.onkeydown=function( e ){
		game.KeyEvent( BBGameEvent.KeyDown,e.keyCode );
		var chr=keyToChar( e.keyCode );
		if( chr ) game.KeyEvent( BBGameEvent.KeyChar,chr );
		if( e.keyCode<48 || (e.keyCode>111 && e.keyCode<122) ) eatEvent( e );
	}

	canvas.onkeyup=function( e ){
		game.KeyEvent( BBGameEvent.KeyUp,e.keyCode );
	}

	canvas.onkeypress=function( e ){
		if( e.charCode ){
			game.KeyEvent( BBGameEvent.KeyChar,e.charCode );
		}else if( e.which ){
			game.KeyEvent( BBGameEvent.KeyChar,e.which );
		}
	}

	canvas.onmousedown=function( e ){
		switch( e.button ){
		case 0:game.MouseEvent( BBGameEvent.MouseDown,0,mouseX(e),mouseY(e) );break;
		case 1:game.MouseEvent( BBGameEvent.MouseDown,2,mouseX(e),mouseY(e) );break;
		case 2:game.MouseEvent( BBGameEvent.MouseDown,1,mouseX(e),mouseY(e) );break;
		}
		eatEvent( e );
	}
	
	canvas.onmouseup=function( e ){
		switch( e.button ){
		case 0:game.MouseEvent( BBGameEvent.MouseUp,0,mouseX(e),mouseY(e) );break;
		case 1:game.MouseEvent( BBGameEvent.MouseUp,2,mouseX(e),mouseY(e) );break;
		case 2:game.MouseEvent( BBGameEvent.MouseUp,1,mouseX(e),mouseY(e) );break;
		}
		eatEvent( e );
	}
	
	canvas.onmousemove=function( e ){
		game.MouseEvent( BBGameEvent.MouseMove,-1,mouseX(e),mouseY(e) );
		eatEvent( e );
	}

	canvas.onmouseout=function( e ){
		game.MouseEvent( BBGameEvent.MouseUp,0,mouseX(e),mouseY(e) );
		game.MouseEvent( BBGameEvent.MouseUp,1,mouseX(e),mouseY(e) );
		game.MouseEvent( BBGameEvent.MouseUp,2,mouseX(e),mouseY(e) );
		eatEvent( e );
	}
	
	canvas.onclick=function( e ){
		if( game.Suspended() ){
			canvas.focus();
		}
		eatEvent( e );
		return;
	}
	
	canvas.oncontextmenu=function( e ){
		return false;
	}
	
	canvas.ontouchstart=function( e ){
		if( game.Suspended() ){
			canvas.focus();
		}
		for( var i=0;i<e.changedTouches.length;++i ){
			var touch=e.changedTouches[i];
			for( var j=0;j<32;++j ){
				if( touchIds[j]!=-1 ) continue;
				touchIds[j]=touch.identifier;
				game.TouchEvent( BBGameEvent.TouchDown,j,touchX(touch),touchY(touch) );
				break;
			}
		}
		eatEvent( e );
	}
	
	canvas.ontouchmove=function( e ){
		for( var i=0;i<e.changedTouches.length;++i ){
			var touch=e.changedTouches[i];
			for( var j=0;j<32;++j ){
				if( touchIds[j]!=touch.identifier ) continue;
				game.TouchEvent( BBGameEvent.TouchMove,j,touchX(touch),touchY(touch) );
				break;
			}
		}
		eatEvent( e );
	}
	
	canvas.ontouchend=function( e ){
		for( var i=0;i<e.changedTouches.length;++i ){
			var touch=e.changedTouches[i];
			for( var j=0;j<32;++j ){
				if( touchIds[j]!=touch.identifier ) continue;
				touchIds[j]=-1;
				game.TouchEvent( BBGameEvent.TouchUp,j,touchX(touch),touchY(touch) );
				break;
			}
		}
		eatEvent( e );
	}
	
	window.ondevicemotion=function( e ){
		var tx=e.accelerationIncludingGravity.x/9.81;
		var ty=e.accelerationIncludingGravity.y/9.81;
		var tz=e.accelerationIncludingGravity.z/9.81;
		var x,y;
		switch( window.orientation ){
		case   0:x=+tx;y=-ty;break;
		case 180:x=-tx;y=+ty;break;
		case  90:x=-ty;y=-tx;break;
		case -90:x=+ty;y=+tx;break;
		}
		game.MotionEvent( BBGameEvent.MotionAccel,0,x,y,tz );
		eatEvent( e );
	}

	canvas.onfocus=function( e ){
		if( CFG_MOJO_AUTO_SUSPEND_ENABLED=="1" ){
			game.ResumeGame();
		}
	}
	
	canvas.onblur=function( e ){
		for( var i=0;i<256;++i ) game.KeyEvent( BBGameEvent.KeyUp,i );
		if( CFG_MOJO_AUTO_SUSPEND_ENABLED=="1" ){
			game.SuspendGame();
		}
	}
	
	canvas.updateSize=function(){
		xscale=canvas.width/canvas.clientWidth;
		yscale=canvas.height/canvas.clientHeight;
		game.RenderGame();
	}
	
	canvas.updateSize();
	
	canvas.focus();
	
	game.StartGame();

	game.RenderGame();
}


function BBMonkeyGame( canvas ){
	BBHtml5Game.call( this,canvas );
}

BBMonkeyGame.prototype=extend_class( BBHtml5Game );

BBMonkeyGame.Main=function( canvas ){

	var game=new BBMonkeyGame( canvas );

	try{

		bbInit();
		bbMain();

	}catch( ex ){
	
		game.Die( ex );
		return;
	}

	if( !game.Delegate() ) return;
	
	game.Run();
}


// HTML5 mojo runtime.
//
// Copyright 2011 Mark Sibly, all rights reserved.
// No warranty implied; use at your own risk.

// ***** gxtkGraphics class *****

function gxtkGraphics(){
	this.game=BBHtml5Game.Html5Game();
	this.canvas=this.game.GetCanvas()
	this.width=this.canvas.width;
	this.height=this.canvas.height;
	this.gl=null;
	this.gc=this.canvas.getContext( '2d' );
	this.tmpCanvas=null;
	this.r=255;
	this.b=255;
	this.g=255;
	this.white=true;
	this.color="rgb(255,255,255)"
	this.alpha=1;
	this.blend="source-over";
	this.ix=1;this.iy=0;
	this.jx=0;this.jy=1;
	this.tx=0;this.ty=0;
	this.tformed=false;
	this.scissorX=0;
	this.scissorY=0;
	this.scissorWidth=0;
	this.scissorHeight=0;
	this.clipped=false;
}

gxtkGraphics.prototype.BeginRender=function(){
	this.width=this.canvas.width;
	this.height=this.canvas.height;
	if( !this.gc ) return 0;
	this.gc.save();
	if( this.game.GetLoading() ) return 2;
	return 1;
}

gxtkGraphics.prototype.EndRender=function(){
	if( this.gc ) this.gc.restore();
}

gxtkGraphics.prototype.Width=function(){
	return this.width;
}

gxtkGraphics.prototype.Height=function(){
	return this.height;
}

gxtkGraphics.prototype.LoadSurface=function( path ){
	var game=this.game;

	var ty=game.GetMetaData( path,"type" );
	if( ty.indexOf( "image/" )!=0 ) return null;
	
	game.IncLoading();

	var image=new Image();
	image.onload=function(){ game.DecLoading(); }
	image.onerror=function(){ game.DecLoading(); }
	image.meta_width=parseInt( game.GetMetaData( path,"width" ) );
	image.meta_height=parseInt( game.GetMetaData( path,"height" ) );
	image.src=game.PathToUrl( path );

	return new gxtkSurface( image,this );
}

gxtkGraphics.prototype.CreateSurface=function( width,height ){
	var canvas=document.createElement( 'canvas' );
	
	canvas.width=width;
	canvas.height=height;
	canvas.meta_width=width;
	canvas.meta_height=height;
	canvas.complete=true;
	
	var surface=new gxtkSurface( canvas,this );
	
	surface.gc=canvas.getContext( '2d' );
	
	return surface;
}

gxtkGraphics.prototype.SetAlpha=function( alpha ){
	this.alpha=alpha;
	this.gc.globalAlpha=alpha;
}

gxtkGraphics.prototype.SetColor=function( r,g,b ){
	this.r=r;
	this.g=g;
	this.b=b;
	this.white=(r==255 && g==255 && b==255);
	this.color="rgb("+(r|0)+","+(g|0)+","+(b|0)+")";
	this.gc.fillStyle=this.color;
	this.gc.strokeStyle=this.color;
}

gxtkGraphics.prototype.SetBlend=function( blend ){
	switch( blend ){
	case 1:
		this.blend="lighter";
		break;
	default:
		this.blend="source-over";
	}
	this.gc.globalCompositeOperation=this.blend;
}

gxtkGraphics.prototype.SetScissor=function( x,y,w,h ){
	this.scissorX=x;
	this.scissorY=y;
	this.scissorWidth=w;
	this.scissorHeight=h;
	this.clipped=(x!=0 || y!=0 || w!=this.canvas.width || h!=this.canvas.height);
	this.gc.restore();
	this.gc.save();
	if( this.clipped ){
		this.gc.beginPath();
		this.gc.rect( x,y,w,h );
		this.gc.clip();
		this.gc.closePath();
	}
	this.gc.fillStyle=this.color;
	this.gc.strokeStyle=this.color;	
	this.gc.globalAlpha=this.alpha;	
	this.gc.globalCompositeOperation=this.blend;
	if( this.tformed ) this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
}

gxtkGraphics.prototype.SetMatrix=function( ix,iy,jx,jy,tx,ty ){
	this.ix=ix;this.iy=iy;
	this.jx=jx;this.jy=jy;
	this.tx=tx;this.ty=ty;
	this.gc.setTransform( ix,iy,jx,jy,tx,ty );
	this.tformed=(ix!=1 || iy!=0 || jx!=0 || jy!=1 || tx!=0 || ty!=0);
}

gxtkGraphics.prototype.Cls=function( r,g,b ){
	if( this.tformed ) this.gc.setTransform( 1,0,0,1,0,0 );
	this.gc.fillStyle="rgb("+(r|0)+","+(g|0)+","+(b|0)+")";
	this.gc.globalAlpha=1;
	this.gc.globalCompositeOperation="source-over";
	this.gc.fillRect( 0,0,this.canvas.width,this.canvas.height );
	this.gc.fillStyle=this.color;
	this.gc.globalAlpha=this.alpha;
	this.gc.globalCompositeOperation=this.blend;
	if( this.tformed ) this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
}

gxtkGraphics.prototype.DrawPoint=function( x,y ){
	if( this.tformed ){
		var px=x;
		x=px * this.ix + y * this.jx + this.tx;
		y=px * this.iy + y * this.jy + this.ty;
		this.gc.setTransform( 1,0,0,1,0,0 );
		this.gc.fillRect( x,y,1,1 );
		this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
	}else{
		this.gc.fillRect( x,y,1,1 );
	}
}

gxtkGraphics.prototype.DrawRect=function( x,y,w,h ){
	if( w<0 ){ x+=w;w=-w; }
	if( h<0 ){ y+=h;h=-h; }
	if( w<=0 || h<=0 ) return;
	//
	this.gc.fillRect( x,y,w,h );
}

gxtkGraphics.prototype.DrawLine=function( x1,y1,x2,y2 ){
	if( this.tformed ){
		var x1_t=x1 * this.ix + y1 * this.jx + this.tx;
		var y1_t=x1 * this.iy + y1 * this.jy + this.ty;
		var x2_t=x2 * this.ix + y2 * this.jx + this.tx;
		var y2_t=x2 * this.iy + y2 * this.jy + this.ty;
		this.gc.setTransform( 1,0,0,1,0,0 );
	  	this.gc.beginPath();
	  	this.gc.moveTo( x1_t,y1_t );
	  	this.gc.lineTo( x2_t,y2_t );
	  	this.gc.stroke();
	  	this.gc.closePath();
		this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
	}else{
	  	this.gc.beginPath();
	  	this.gc.moveTo( x1,y1 );
	  	this.gc.lineTo( x2,y2 );
	  	this.gc.stroke();
	  	this.gc.closePath();
	}
}

gxtkGraphics.prototype.DrawOval=function( x,y,w,h ){
	if( w<0 ){ x+=w;w=-w; }
	if( h<0 ){ y+=h;h=-h; }
	if( w<=0 || h<=0 ) return;
	//
  	var w2=w/2,h2=h/2;
	this.gc.save();
	this.gc.translate( x+w2,y+h2 );
	this.gc.scale( w2,h2 );
  	this.gc.beginPath();
	this.gc.arc( 0,0,1,0,Math.PI*2,false );
	this.gc.fill();
  	this.gc.closePath();
	this.gc.restore();
}

gxtkGraphics.prototype.DrawPoly=function( verts ){
	if( verts.length<2 ) return;
	this.gc.beginPath();
	this.gc.moveTo( verts[0],verts[1] );
	for( var i=2;i<verts.length;i+=2 ){
		this.gc.lineTo( verts[i],verts[i+1] );
	}
	this.gc.fill();
	this.gc.closePath();
}

gxtkGraphics.prototype.DrawPoly2=function( verts,surface,srx,srcy ){
	if( verts.length<4 ) return;
	this.gc.beginPath();
	this.gc.moveTo( verts[0],verts[1] );
	for( var i=4;i<verts.length;i+=4 ){
		this.gc.lineTo( verts[i],verts[i+1] );
	}
	this.gc.fill();
	this.gc.closePath();
}

gxtkGraphics.prototype.DrawSurface=function( surface,x,y ){
	if( !surface.image.complete ) return;
	
	if( this.white ){
		this.gc.drawImage( surface.image,x,y );
		return;
	}
	
	this.DrawImageTinted( surface.image,x,y,0,0,surface.swidth,surface.sheight );
}

gxtkGraphics.prototype.DrawSurface2=function( surface,x,y,srcx,srcy,srcw,srch ){
	if( !surface.image.complete ) return;

	if( srcw<0 ){ srcx+=srcw;srcw=-srcw; }
	if( srch<0 ){ srcy+=srch;srch=-srch; }
	if( srcw<=0 || srch<=0 ) return;

	if( this.white ){
		this.gc.drawImage( surface.image,srcx,srcy,srcw,srch,x,y,srcw,srch );
		return;
	}
	
	this.DrawImageTinted( surface.image,x,y,srcx,srcy,srcw,srch  );
}

gxtkGraphics.prototype.DrawImageTinted=function( image,dx,dy,sx,sy,sw,sh ){

	if( !this.tmpCanvas ){
		this.tmpCanvas=document.createElement( "canvas" );
	}

	if( sw>this.tmpCanvas.width || sh>this.tmpCanvas.height ){
		this.tmpCanvas.width=Math.max( sw,this.tmpCanvas.width );
		this.tmpCanvas.height=Math.max( sh,this.tmpCanvas.height );
	}
	
	var tmpGC=this.tmpCanvas.getContext( "2d" );
	tmpGC.globalCompositeOperation="copy";
	
	tmpGC.drawImage( image,sx,sy,sw,sh,0,0,sw,sh );
	
	var imgData=tmpGC.getImageData( 0,0,sw,sh );
	
	var p=imgData.data,sz=sw*sh*4,i;
	
	for( i=0;i<sz;i+=4 ){
		p[i]=p[i]*this.r/255;
		p[i+1]=p[i+1]*this.g/255;
		p[i+2]=p[i+2]*this.b/255;
	}
	
	tmpGC.putImageData( imgData,0,0 );
	
	this.gc.drawImage( this.tmpCanvas,0,0,sw,sh,dx,dy,sw,sh );
}

gxtkGraphics.prototype.ReadPixels=function( pixels,x,y,width,height,offset,pitch ){

	var imgData=this.gc.getImageData( x,y,width,height );
	
	var p=imgData.data,i=0,j=offset,px,py;
	
	for( py=0;py<height;++py ){
		for( px=0;px<width;++px ){
			pixels[j++]=(p[i+3]<<24)|(p[i]<<16)|(p[i+1]<<8)|p[i+2];
			i+=4;
		}
		j+=pitch-width;
	}
}

gxtkGraphics.prototype.WritePixels2=function( surface,pixels,x,y,width,height,offset,pitch ){

	if( !surface.gc ){
		if( !surface.image.complete ) return;
		var canvas=document.createElement( "canvas" );
		canvas.width=surface.swidth;
		canvas.height=surface.sheight;
		surface.gc=canvas.getContext( "2d" );
		surface.gc.globalCompositeOperation="copy";
		surface.gc.drawImage( surface.image,0,0 );
		surface.image=canvas;
	}

	var imgData=surface.gc.createImageData( width,height );

	var p=imgData.data,i=0,j=offset,px,py,argb;
	
	for( py=0;py<height;++py ){
		for( px=0;px<width;++px ){
			argb=pixels[j++];
			p[i]=(argb>>16) & 0xff;
			p[i+1]=(argb>>8) & 0xff;
			p[i+2]=argb & 0xff;
			p[i+3]=(argb>>24) & 0xff;
			i+=4;
		}
		j+=pitch-width;
	}
	
	surface.gc.putImageData( imgData,x,y );
}

// ***** gxtkSurface class *****

function gxtkSurface( image,graphics ){
	this.image=image;
	this.graphics=graphics;
	this.swidth=image.meta_width;
	this.sheight=image.meta_height;
}

// ***** GXTK API *****

gxtkSurface.prototype.Discard=function(){
	if( this.image ){
		this.image=null;
	}
}

gxtkSurface.prototype.Width=function(){
	return this.swidth;
}

gxtkSurface.prototype.Height=function(){
	return this.sheight;
}

gxtkSurface.prototype.Loaded=function(){
	return this.image.complete;
}

gxtkSurface.prototype.OnUnsafeLoadComplete=function(){
}

if( CFG_HTML5_WEBAUDIO_ENABLED=="1" && (window.AudioContext || window.webkitAudioContext) ){

//print( "Using WebAudio!" );

// ***** WebAudio *****

var wa=null;

// ***** WebAudio gxtkSample *****

var gxtkSample=function(){
	this.waBuffer=null;
	this.state=0;
}

gxtkSample.prototype.Load=function( path ){
	if( this.state ) return false;

	var req=new XMLHttpRequest();
	
	req.open( "get",BBGame.Game().PathToUrl( path ),true );
	req.responseType="arraybuffer";
	
	var abuf=this;
	
	req.onload=function(){
		wa.decodeAudioData( req.response,function( buffer ){
			//success!
			abuf.waBuffer=buffer;
			abuf.state=1;
		},function(){
			abuf.state=-1;
		} );
	}
	
	req.onerror=function(){
		abuf.state=-1;
	}
	
	req.send();
	
	this.state=2;
			
	return true;
}

gxtkSample.prototype.Discard=function(){
}

// ***** WebAudio gxtkChannel *****

var gxtkChannel=function(){
	this.buffer=null;
	this.flags=0;
	this.volume=1;
	this.pan=0;
	this.rate=1;
	this.waSource=null;
	this.waPan=wa.create
	this.waGain=wa.createGain();
	this.waGain.connect( wa.destination );
	this.waPanner=wa.createPanner();
	this.waPanner.rolloffFactor=0;
	this.waPanner.connect( this.waGain );
	this.startTime=0;
	this.offset=0;
	this.state=0;
}

// ***** WebAudio gxtkAudio *****

var gxtkAudio=function(){

	if( !wa ){
		window.AudioContext=window.AudioContext || window.webkitAudioContext;
		wa=new AudioContext();
	}
	
	this.okay=true;
	this.music=null;
	this.musicState=0;
	this.musicVolume=1;
	this.channels=new Array();
	for( var i=0;i<32;++i ){
		this.channels[i]=new gxtkChannel();
	}
}

gxtkAudio.prototype.Suspend=function(){
	if( this.MusicState()==1 ) this.music.pause();
	for( var i=0;i<32;++i ){
		var chan=this.channels[i];
		if( chan.state!=1 ) continue;
		this.PauseChannel( i );
		chan.state=5;
	}
}

gxtkAudio.prototype.Resume=function(){
	if( this.MusicState()==1 ) this.music.play();
	for( var i=0;i<32;++i ){
		var chan=this.channels[i];
		if( chan.state!=5 ) continue;
		chan.state=2;
		this.ResumeChannel( i );
	}
}

gxtkAudio.prototype.LoadSample=function( path ){

	var sample=new gxtkSample();
	if( !sample.Load( BBHtml5Game.Html5Game().PathToUrl( path ) ) ) return null;
	
	return sample;
}

gxtkAudio.prototype.PlaySample=function( buffer,channel,flags ){

	if( buffer.state!=1 ) return;

	var chan=this.channels[channel];
	
	if( chan.state ){
		chan.waSource.onended=null
		chan.waSource.stop( 0 );
	}
	
	chan.buffer=buffer;
	chan.flags=flags;

	chan.waSource=wa.createBufferSource();
	chan.waSource.buffer=buffer.waBuffer;
	chan.waSource.playbackRate.value=chan.rate;
	chan.waSource.loop=(flags&1)!=0;
	chan.waSource.connect( chan.waPanner );
	
	chan.waSource.onended=function( e ){
		chan.waSource=null;
		chan.state=0;
	}

	chan.offset=0;	
	chan.startTime=wa.currentTime;
	chan.waSource.start( 0 );

	chan.state=1;
}

gxtkAudio.prototype.StopChannel=function( channel ){

	var chan=this.channels[channel];
	if( !chan.state ) return;
	
	if( chan.state==1 ){
		chan.waSource.onended=null;
		chan.waSource.stop( 0 );
		chan.waSource=null;
	}

	chan.state=0;
}

gxtkAudio.prototype.PauseChannel=function( channel ){

	var chan=this.channels[channel];
	if( chan.state!=1 ) return;
	
	chan.offset=(chan.offset+(wa.currentTime-chan.startTime)*chan.rate)%chan.buffer.waBuffer.duration;
	
	chan.waSource.onended=null;
	chan.waSource.stop( 0 );
	chan.waSource=null;
	
	chan.state=2;
}

gxtkAudio.prototype.ResumeChannel=function( channel ){

	var chan=this.channels[channel];
	if( chan.state!=2 ) return;
	
	chan.waSource=wa.createBufferSource();
	chan.waSource.buffer=chan.buffer.waBuffer;
	chan.waSource.playbackRate.value=chan.rate;
	chan.waSource.loop=(chan.flags&1)!=0;
	chan.waSource.connect( chan.waPanner );
	
	chan.waSource.onended=function( e ){
		chan.waSource=null;
		chan.state=0;
	}
	
	chan.startTime=wa.currentTime;
	chan.waSource.start( 0,chan.offset );

	chan.state=1;
}

gxtkAudio.prototype.ChannelState=function( channel ){
	return this.channels[channel].state & 3;
}

gxtkAudio.prototype.SetVolume=function( channel,volume ){
	var chan=this.channels[channel];

	chan.volume=volume;
	
	chan.waGain.gain.value=volume;
}

gxtkAudio.prototype.SetPan=function( channel,pan ){
	var chan=this.channels[channel];

	chan.pan=pan;
	
	var sin=Math.sin( pan*3.14159265359/2 );
	var cos=Math.cos( pan*3.14159265359/2 );
	
	chan.waPanner.setPosition( sin,0,-cos );
}

gxtkAudio.prototype.SetRate=function( channel,rate ){

	var chan=this.channels[channel];

	if( chan.state==1 ){
		//update offset for pause/resume
		var time=wa.currentTime;
		chan.offset=(chan.offset+(time-chan.startTime)*chan.rate)%chan.buffer.waBuffer.duration;
		chan.startTime=time;
	}

	chan.rate=rate;
	
	if( chan.waSource ) chan.waSource.playbackRate.value=rate;
}

gxtkAudio.prototype.PlayMusic=function( path,flags ){
	if( this.musicState ) this.music.pause();
	this.music=new Audio( BBGame.Game().PathToUrl( path ) );
	this.music.loop=(flags&1)!=0;
	this.music.play();
	this.musicState=1;
}

gxtkAudio.prototype.StopMusic=function(){
	if( !this.musicState ) return;
	this.music.pause();
	this.music=null;
	this.musicState=0;
}

gxtkAudio.prototype.PauseMusic=function(){
	if( this.musicState!=1 ) return;
	this.music.pause();
	this.musicState=2;
}

gxtkAudio.prototype.ResumeMusic=function(){
	if( this.musicState!=2 ) return;
	this.music.play();
	this.musicState=1;
}

gxtkAudio.prototype.MusicState=function(){
	if( this.musicState==1 && this.music.ended && !this.music.loop ){
		this.music=null;
		this.musicState=0;
	}
	return this.musicState;
}

gxtkAudio.prototype.SetMusicVolume=function( volume ){
	this.musicVolume=volume;
	if( this.musicState ) this.music.volume=volume;
}

}else{

//print( "Using OldAudio!" );

// ***** gxtkChannel class *****

var gxtkChannel=function(){
	this.sample=null;
	this.audio=null;
	this.volume=1;
	this.pan=0;
	this.rate=1;
	this.flags=0;
	this.state=0;
}

// ***** gxtkAudio class *****

var gxtkAudio=function(){
	this.game=BBHtml5Game.Html5Game();
	this.okay=typeof(Audio)!="undefined";
	this.music=null;
	this.channels=new Array(33);
	for( var i=0;i<33;++i ){
		this.channels[i]=new gxtkChannel();
		if( !this.okay ) this.channels[i].state=-1;
	}
}

gxtkAudio.prototype.Suspend=function(){
	var i;
	for( i=0;i<33;++i ){
		var chan=this.channels[i];
		if( chan.state==1 ){
			if( chan.audio.ended && !chan.audio.loop ){
				chan.state=0;
			}else{
				chan.audio.pause();
				chan.state=3;
			}
		}
	}
}

gxtkAudio.prototype.Resume=function(){
	var i;
	for( i=0;i<33;++i ){
		var chan=this.channels[i];
		if( chan.state==3 ){
			chan.audio.play();
			chan.state=1;
		}
	}
}

gxtkAudio.prototype.LoadSample=function( path ){
	if( !this.okay ) return null;

	var audio=new Audio( this.game.PathToUrl( path ) );
	if( !audio ) return null;
	
	return new gxtkSample( audio );
}

gxtkAudio.prototype.PlaySample=function( sample,channel,flags ){
	if( !this.okay ) return;
	
	var chan=this.channels[channel];

	if( chan.state>0 ){
		chan.audio.pause();
		chan.state=0;
	}
	
	for( var i=0;i<33;++i ){
		var chan2=this.channels[i];
		if( chan2.state==1 && chan2.audio.ended && !chan2.audio.loop ) chan.state=0;
		if( chan2.state==0 && chan2.sample ){
			chan2.sample.FreeAudio( chan2.audio );
			chan2.sample=null;
			chan2.audio=null;
		}
	}

	var audio=sample.AllocAudio();
	if( !audio ) return;

	audio.loop=(flags&1)!=0;
	audio.volume=chan.volume;
	audio.play();

	chan.sample=sample;
	chan.audio=audio;
	chan.flags=flags;
	chan.state=1;
}

gxtkAudio.prototype.StopChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state>0 ){
		chan.audio.pause();
		chan.state=0;
	}
}

gxtkAudio.prototype.PauseChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state==1 ){
		if( chan.audio.ended && !chan.audio.loop ){
			chan.state=0;
		}else{
			chan.audio.pause();
			chan.state=2;
		}
	}
}

gxtkAudio.prototype.ResumeChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state==2 ){
		chan.audio.play();
		chan.state=1;
	}
}

gxtkAudio.prototype.ChannelState=function( channel ){
	var chan=this.channels[channel];
	if( chan.state==1 && chan.audio.ended && !chan.audio.loop ) chan.state=0;
	if( chan.state==3 ) return 1;
	return chan.state;
}

gxtkAudio.prototype.SetVolume=function( channel,volume ){
	var chan=this.channels[channel];
	if( chan.state>0 ) chan.audio.volume=volume;
	chan.volume=volume;
}

gxtkAudio.prototype.SetPan=function( channel,pan ){
	var chan=this.channels[channel];
	chan.pan=pan;
}

gxtkAudio.prototype.SetRate=function( channel,rate ){
	var chan=this.channels[channel];
	chan.rate=rate;
}

gxtkAudio.prototype.PlayMusic=function( path,flags ){
	this.StopMusic();
	
	this.music=this.LoadSample( path );
	if( !this.music ) return;
	
	this.PlaySample( this.music,32,flags );
}

gxtkAudio.prototype.StopMusic=function(){
	this.StopChannel( 32 );

	if( this.music ){
		this.music.Discard();
		this.music=null;
	}
}

gxtkAudio.prototype.PauseMusic=function(){
	this.PauseChannel( 32 );
}

gxtkAudio.prototype.ResumeMusic=function(){
	this.ResumeChannel( 32 );
}

gxtkAudio.prototype.MusicState=function(){
	return this.ChannelState( 32 );
}

gxtkAudio.prototype.SetMusicVolume=function( volume ){
	this.SetVolume( 32,volume );
}

// ***** gxtkSample class *****

//function gxtkSample( audio ){
var gxtkSample=function( audio ){
	this.audio=audio;
	this.free=new Array();
	this.insts=new Array();
}

gxtkSample.prototype.FreeAudio=function( audio ){
	this.free.push( audio );
}

gxtkSample.prototype.AllocAudio=function(){
	var audio;
	while( this.free.length ){
		audio=this.free.pop();
		try{
			audio.currentTime=0;
			return audio;
		}catch( ex ){
//			print( "AUDIO ERROR1!" );
		}
	}
	
	//Max out?
	if( this.insts.length==8 ) return null;
	
	audio=new Audio( this.audio.src );
	
	//yucky loop handler for firefox!
	//
	audio.addEventListener( 'ended',function(){
		if( this.loop ){
			try{
				this.currentTime=0;
				this.play();
			}catch( ex ){
//				print( "AUDIO ERROR2!" );
			}
		}
	},false );

	this.insts.push( audio );
	return audio;
}

gxtkSample.prototype.Discard=function(){
}

}


function BBThread(){
	this.result=null;
	this.running=false;
}

BBThread.prototype.Start=function(){
	this.result=null;
	this.running=true;
	this.Run__UNSAFE__();
}

BBThread.prototype.IsRunning=function(){
	return this.running;
}

BBThread.prototype.Result=function(){
	return this.result;
}

BBThread.prototype.Run__UNSAFE__=function(){
	this.running=false;
}


function BBAsyncImageLoaderThread(){
	this._running=false;
}

BBAsyncImageLoaderThread.prototype.Start=function(){

	var thread=this;

	thread._surface=null;
	thread._result=false;
	thread._running=true;

	var image=new Image();

	image.onload=function( e ){
		image.meta_width=image.width;
		image.meta_height=image.height;
		thread._surface=new gxtkSurface( image,thread._device )
		thread._result=true;
		thread._running=false;
	}
	
	image.onerror=function( e ){
		thread._running=false;
	}
	
	image.src=BBGame.Game().PathToUrl( thread._path );
}

BBAsyncImageLoaderThread.prototype.IsRunning=function(){
	return this._running;
}



function BBAsyncSoundLoaderThread(){
	this._running=false;
}
  
if( CFG_HTML5_WEBAUDIO_ENABLED=="1" && (window.AudioContext || window.webkitAudioContext) ){

BBAsyncSoundLoaderThread.prototype.Start=function(){

	this._sample=null;
	if( !this._device.okay ) return;
	
	var thread=this;
	
	thread._sample=null;
	thread._result=false;
	thread._running=true;

	var req=new XMLHttpRequest();
	req.open( "get",BBGame.Game().PathToUrl( this._path ),true );
	req.responseType="arraybuffer";
	
	req.onload=function(){
		//load success!
		wa.decodeAudioData( req.response,function( buffer ){
			//decode success!
			thread._sample=new gxtkSample();
			thread._sample.waBuffer=buffer;
			thread._sample.state=1;
			thread._result=true;
			thread._running=false;
		},function(){	
			//decode fail!
			thread._running=false;
		} );
	}
	
	req.onerror=function(){
		//load fail!
		thread._running=false;
	}
	
	req.send();
}
	
}else{
 
BBAsyncSoundLoaderThread.prototype.Start=function(){

	this._sample=null;
	if( !this._device.okay ) return;
	
	var audio=new Audio();
	if( !audio ) return;
	
	var thread=this;
	
	thread._sample=null;
	thread._result=false;
	thread._running=true;

	audio.src=BBGame.Game().PathToUrl( this._path );
	audio.preload='auto';	
	
	var success=function( e ){
		thread._sample=new gxtkSample( audio );
		thread._result=true;
		thread._running=false;
		audio.removeEventListener( 'canplaythrough',success,false );
		audio.removeEventListener( 'error',error,false );
	}
	
	var error=function( e ){
		thread._running=false;
		audio.removeEventListener( 'canplaythrough',success,false );
		audio.removeEventListener( 'error',error,false );
	}
	
	audio.addEventListener( 'canplaythrough',success,false );
	audio.addEventListener( 'error',error,false );
	
	//voodoo fix for Chrome!
	var timer=setInterval( function(){ if( !thread._running ) clearInterval( timer ); },200 );
	
	audio.load();
}

}
  
BBAsyncSoundLoaderThread.prototype.IsRunning=function(){
	return this._running;
}

function c_App(){
	Object.call(this);
}
c_App.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<152>";
	if((bb_app__app)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<152>";
		error("App has already been created");
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<153>";
	bb_app__app=this;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<154>";
	bb_app__delegate=c_GameDelegate.m_new.call(new c_GameDelegate);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<155>";
	bb_app__game.SetDelegate(bb_app__delegate);
	pop_err();
	return this;
}
c_App.prototype.p_OnResize=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnCreate=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnSuspend=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnResume=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnUpdate=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnLoading=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnRender=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnClose=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<177>";
	bb_app_EndApp();
	pop_err();
	return 0;
}
c_App.prototype.p_OnBack=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<181>";
	this.p_OnClose();
	pop_err();
	return 0;
}
function c_iApp(){
	c_App.call(this);
}
c_iApp.prototype=extend_class(c_App);
c_iApp.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<276>";
	c_App.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<276>";
	pop_err();
	return this;
}
c_iApp.prototype.p_OnBack=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<279>";
	bb_app2_iBack();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<280>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnClose=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<284>";
	bb_app2_iClose();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<285>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnLoading=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<289>";
	bb_app2_iLoading();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<290>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<294>";
	bb_app2_iRender();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<295>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnResize=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<299>";
	bb_app2_iResize();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<300>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnResume=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<304>";
	bb_app2_iResume();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<305>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnSuspend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<309>";
	bb_app2_iSuspend();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<310>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<314>";
	bb_app2_iUpdate();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<315>";
	pop_err();
	return 0;
}
function c_Game(){
	c_iApp.call(this);
}
c_Game.prototype=extend_class(c_iApp);
c_Game.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<16>";
	c_iApp.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<16>";
	pop_err();
	return this;
}
c_Game.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<23>";
	bb_random_Seed=bb_app_Millisecs();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<31>";
	bb_main_gfxMonkey=bb_gfx_iLoadSprite("char_walk_down.png",1);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<32>";
	bb_main_menu=c_MenuScene.m_new.call(new c_MenuScene);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<33>";
	bb_main_gameplay=c_GameplayScene.m_new.call(new c_GameplayScene);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<34>";
	bb_app2_iStart2((bb_main_menu),60);
	pop_err();
	return 0;
}
var bb_app__app=null;
function c_GameDelegate(){
	BBGameDelegate.call(this);
	this.m__graphics=null;
	this.m__audio=null;
	this.m__input=null;
}
c_GameDelegate.prototype=extend_class(BBGameDelegate);
c_GameDelegate.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<65>";
	pop_err();
	return this;
}
c_GameDelegate.prototype.StartGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<75>";
	this.m__graphics=(new gxtkGraphics);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<76>";
	bb_graphics_SetGraphicsDevice(this.m__graphics);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<77>";
	bb_graphics_SetFont(null,32);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<79>";
	this.m__audio=(new gxtkAudio);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<80>";
	bb_audio_SetAudioDevice(this.m__audio);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<82>";
	this.m__input=c_InputDevice.m_new.call(new c_InputDevice);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<83>";
	bb_input_SetInputDevice(this.m__input);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<85>";
	bb_app_ValidateDeviceWindow(false);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<87>";
	bb_app_EnumDisplayModes();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<89>";
	bb_app__app.p_OnCreate();
	pop_err();
}
c_GameDelegate.prototype.SuspendGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<93>";
	bb_app__app.p_OnSuspend();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<94>";
	this.m__audio.Suspend();
	pop_err();
}
c_GameDelegate.prototype.ResumeGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<98>";
	this.m__audio.Resume();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<99>";
	bb_app__app.p_OnResume();
	pop_err();
}
c_GameDelegate.prototype.UpdateGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<103>";
	bb_app_ValidateDeviceWindow(true);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<104>";
	this.m__input.p_BeginUpdate();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<105>";
	bb_app__app.p_OnUpdate();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<106>";
	this.m__input.p_EndUpdate();
	pop_err();
}
c_GameDelegate.prototype.RenderGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<110>";
	bb_app_ValidateDeviceWindow(true);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<111>";
	var t_mode=this.m__graphics.BeginRender();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<112>";
	if((t_mode)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<112>";
		bb_graphics_BeginRender();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<113>";
	if(t_mode==2){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<113>";
		bb_app__app.p_OnLoading();
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<113>";
		bb_app__app.p_OnRender();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<114>";
	if((t_mode)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<114>";
		bb_graphics_EndRender();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<115>";
	this.m__graphics.EndRender();
	pop_err();
}
c_GameDelegate.prototype.KeyEvent=function(t_event,t_data){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<119>";
	this.m__input.p_KeyEvent(t_event,t_data);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<120>";
	if(t_event!=1){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<121>";
	var t_1=t_data;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<122>";
	if(t_1==432){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<123>";
		bb_app__app.p_OnClose();
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<124>";
		if(t_1==416){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<125>";
			bb_app__app.p_OnBack();
		}
	}
	pop_err();
}
c_GameDelegate.prototype.MouseEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<130>";
	this.m__input.p_MouseEvent(t_event,t_data,t_x,t_y);
	pop_err();
}
c_GameDelegate.prototype.TouchEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<134>";
	this.m__input.p_TouchEvent(t_event,t_data,t_x,t_y);
	pop_err();
}
c_GameDelegate.prototype.MotionEvent=function(t_event,t_data,t_x,t_y,t_z){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<138>";
	this.m__input.p_MotionEvent(t_event,t_data,t_x,t_y,t_z);
	pop_err();
}
c_GameDelegate.prototype.DiscardGraphics=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<142>";
	this.m__graphics.DiscardGraphics();
	pop_err();
}
var bb_app__delegate=null;
var bb_app__game=null;
function bbMain(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<216>";
	var t_g=c_Game.m_new.call(new c_Game);
	pop_err();
	return 0;
}
var bb_graphics_device=null;
function bb_graphics_SetGraphicsDevice(t_dev){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<63>";
	bb_graphics_device=t_dev;
	pop_err();
	return 0;
}
function c_Image(){
	Object.call(this);
	this.m_surface=null;
	this.m_width=0;
	this.m_height=0;
	this.m_frames=[];
	this.m_flags=0;
	this.m_tx=.0;
	this.m_ty=.0;
	this.m_source=null;
}
c_Image.m_DefaultFlags=0;
c_Image.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<70>";
	pop_err();
	return this;
}
c_Image.prototype.p_SetHandle=function(t_tx,t_ty){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<114>";
	dbg_object(this).m_tx=t_tx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<115>";
	dbg_object(this).m_ty=t_ty;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<116>";
	dbg_object(this).m_flags=dbg_object(this).m_flags&-2;
	pop_err();
	return 0;
}
c_Image.prototype.p_ApplyFlags=function(t_iflags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<197>";
	this.m_flags=t_iflags;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<199>";
	if((this.m_flags&2)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<200>";
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<200>";
		var t_=this.m_frames;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<200>";
		var t_2=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<200>";
		while(t_2<t_.length){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<200>";
			var t_f=dbg_array(t_,t_2)[dbg_index];
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<200>";
			t_2=t_2+1;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<201>";
			dbg_object(t_f).m_x+=1;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<203>";
		this.m_width-=2;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<206>";
	if((this.m_flags&4)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<207>";
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<207>";
		var t_3=this.m_frames;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<207>";
		var t_4=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<207>";
		while(t_4<t_3.length){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<207>";
			var t_f2=dbg_array(t_3,t_4)[dbg_index];
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<207>";
			t_4=t_4+1;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<208>";
			dbg_object(t_f2).m_y+=1;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<210>";
		this.m_height-=2;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<213>";
	if((this.m_flags&1)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<214>";
		this.p_SetHandle((this.m_width)/2.0,(this.m_height)/2.0);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<217>";
	if(this.m_frames.length==1 && dbg_object(dbg_array(this.m_frames,0)[dbg_index]).m_x==0 && dbg_object(dbg_array(this.m_frames,0)[dbg_index]).m_y==0 && this.m_width==this.m_surface.Width() && this.m_height==this.m_surface.Height()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<218>";
		this.m_flags|=65536;
	}
	pop_err();
	return 0;
}
c_Image.prototype.p_Init=function(t_surf,t_nframes,t_iflags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<143>";
	if((this.m_surface)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<143>";
		error("Image already initialized");
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<144>";
	this.m_surface=t_surf;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<146>";
	this.m_width=((this.m_surface.Width()/t_nframes)|0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<147>";
	this.m_height=this.m_surface.Height();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<149>";
	this.m_frames=new_object_array(t_nframes);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<150>";
	for(var t_i=0;t_i<t_nframes;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<151>";
		dbg_array(this.m_frames,t_i)[dbg_index]=c_Frame.m_new.call(new c_Frame,t_i*this.m_width,0);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<154>";
	this.p_ApplyFlags(t_iflags);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<155>";
	pop_err();
	return this;
}
c_Image.prototype.p_Init2=function(t_surf,t_x,t_y,t_iwidth,t_iheight,t_nframes,t_iflags,t_src,t_srcx,t_srcy,t_srcw,t_srch){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<159>";
	if((this.m_surface)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<159>";
		error("Image already initialized");
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<160>";
	this.m_surface=t_surf;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<161>";
	this.m_source=t_src;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<163>";
	this.m_width=t_iwidth;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<164>";
	this.m_height=t_iheight;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<166>";
	this.m_frames=new_object_array(t_nframes);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<168>";
	var t_ix=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<168>";
	var t_iy=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<170>";
	for(var t_i=0;t_i<t_nframes;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<171>";
		if(t_ix+this.m_width>t_srcw){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<172>";
			t_ix=0;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<173>";
			t_iy+=this.m_height;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<175>";
		if(t_ix+this.m_width>t_srcw || t_iy+this.m_height>t_srch){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<176>";
			error("Image frame outside surface");
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<178>";
		dbg_array(this.m_frames,t_i)[dbg_index]=c_Frame.m_new.call(new c_Frame,t_ix+t_srcx,t_iy+t_srcy);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<179>";
		t_ix+=this.m_width;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<182>";
	this.p_ApplyFlags(t_iflags);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<183>";
	pop_err();
	return this;
}
c_Image.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<81>";
	pop_err();
	return this.m_width;
}
c_Image.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<85>";
	pop_err();
	return this.m_height;
}
c_Image.prototype.p_Frames=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<93>";
	var t_=this.m_frames.length;
	pop_err();
	return t_;
}
function c_GraphicsContext(){
	Object.call(this);
	this.m_defaultFont=null;
	this.m_font=null;
	this.m_firstChar=0;
	this.m_matrixSp=0;
	this.m_ix=1.0;
	this.m_iy=.0;
	this.m_jx=.0;
	this.m_jy=1.0;
	this.m_tx=.0;
	this.m_ty=.0;
	this.m_tformed=0;
	this.m_matDirty=0;
	this.m_color_r=.0;
	this.m_color_g=.0;
	this.m_color_b=.0;
	this.m_alpha=.0;
	this.m_blend=0;
	this.m_scissor_x=.0;
	this.m_scissor_y=.0;
	this.m_scissor_width=.0;
	this.m_scissor_height=.0;
	this.m_matrixStack=new_number_array(192);
}
c_GraphicsContext.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<29>";
	pop_err();
	return this;
}
c_GraphicsContext.prototype.p_Validate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<40>";
	if((this.m_matDirty)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<41>";
		bb_graphics_renderDevice.SetMatrix(dbg_object(bb_graphics_context).m_ix,dbg_object(bb_graphics_context).m_iy,dbg_object(bb_graphics_context).m_jx,dbg_object(bb_graphics_context).m_jy,dbg_object(bb_graphics_context).m_tx,dbg_object(bb_graphics_context).m_ty);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<42>";
		this.m_matDirty=0;
	}
	pop_err();
	return 0;
}
var bb_graphics_context=null;
function bb_data_FixDataPath(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/data.monkey<7>";
	var t_i=t_path.indexOf(":/",0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/data.monkey<8>";
	if(t_i!=-1 && t_path.indexOf("/",0)==t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/data.monkey<8>";
		pop_err();
		return t_path;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/data.monkey<9>";
	if(string_startswith(t_path,"./") || string_startswith(t_path,"/")){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/data.monkey<9>";
		pop_err();
		return t_path;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/data.monkey<10>";
	var t_="monkey://data/"+t_path;
	pop_err();
	return t_;
}
function c_Frame(){
	Object.call(this);
	this.m_x=0;
	this.m_y=0;
}
c_Frame.m_new=function(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<23>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<24>";
	dbg_object(this).m_y=t_y;
	pop_err();
	return this;
}
c_Frame.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<18>";
	pop_err();
	return this;
}
function bb_graphics_LoadImage(t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<239>";
	var t_surf=bb_graphics_device.LoadSurface(bb_data_FixDataPath(t_path));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<240>";
	if((t_surf)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<240>";
		var t_=(c_Image.m_new.call(new c_Image)).p_Init(t_surf,t_frameCount,t_flags);
		pop_err();
		return t_;
	}
	pop_err();
	return null;
}
function bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<244>";
	var t_surf=bb_graphics_device.LoadSurface(bb_data_FixDataPath(t_path));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<245>";
	if((t_surf)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<245>";
		var t_=(c_Image.m_new.call(new c_Image)).p_Init2(t_surf,0,0,t_frameWidth,t_frameHeight,t_frameCount,t_flags,null,0,0,t_surf.Width(),t_surf.Height());
		pop_err();
		return t_;
	}
	pop_err();
	return null;
}
function bb_graphics_SetFont(t_font,t_firstChar){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<548>";
	if(!((t_font)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<549>";
		if(!((dbg_object(bb_graphics_context).m_defaultFont)!=null)){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<550>";
			dbg_object(bb_graphics_context).m_defaultFont=bb_graphics_LoadImage("mojo_font.png",96,2);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<552>";
		t_font=dbg_object(bb_graphics_context).m_defaultFont;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<553>";
		t_firstChar=32;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<555>";
	dbg_object(bb_graphics_context).m_font=t_font;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<556>";
	dbg_object(bb_graphics_context).m_firstChar=t_firstChar;
	pop_err();
	return 0;
}
var bb_audio_device=null;
function bb_audio_SetAudioDevice(t_dev){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/audio.monkey<22>";
	bb_audio_device=t_dev;
	pop_err();
	return 0;
}
function c_InputDevice(){
	Object.call(this);
	this.m__joyStates=new_object_array(4);
	this.m__keyDown=new_bool_array(512);
	this.m__keyHitPut=0;
	this.m__keyHitQueue=new_number_array(33);
	this.m__keyHit=new_number_array(512);
	this.m__charGet=0;
	this.m__charPut=0;
	this.m__charQueue=new_number_array(32);
	this.m__mouseX=.0;
	this.m__mouseY=.0;
	this.m__touchX=new_number_array(32);
	this.m__touchY=new_number_array(32);
	this.m__accelX=.0;
	this.m__accelY=.0;
	this.m__accelZ=.0;
}
c_InputDevice.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<26>";
	for(var t_i=0;t_i<4;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<27>";
		dbg_array(this.m__joyStates,t_i)[dbg_index]=c_JoyState.m_new.call(new c_JoyState);
	}
	pop_err();
	return this;
}
c_InputDevice.prototype.p_PutKeyHit=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<237>";
	if(this.m__keyHitPut==this.m__keyHitQueue.length){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<238>";
	dbg_array(this.m__keyHit,t_key)[dbg_index]+=1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<239>";
	dbg_array(this.m__keyHitQueue,this.m__keyHitPut)[dbg_index]=t_key;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<240>";
	this.m__keyHitPut+=1;
	pop_err();
}
c_InputDevice.prototype.p_BeginUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<189>";
	for(var t_i=0;t_i<4;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<190>";
		var t_state=dbg_array(this.m__joyStates,t_i)[dbg_index];
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<191>";
		if(!BBGame.Game().PollJoystick(t_i,dbg_object(t_state).m_joyx,dbg_object(t_state).m_joyy,dbg_object(t_state).m_joyz,dbg_object(t_state).m_buttons)){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<191>";
			break;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<192>";
		for(var t_j=0;t_j<32;t_j=t_j+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<193>";
			var t_key=256+t_i*32+t_j;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<194>";
			if(dbg_array(dbg_object(t_state).m_buttons,t_j)[dbg_index]){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<195>";
				if(!dbg_array(this.m__keyDown,t_key)[dbg_index]){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<196>";
					dbg_array(this.m__keyDown,t_key)[dbg_index]=true;
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<197>";
					this.p_PutKeyHit(t_key);
				}
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<200>";
				dbg_array(this.m__keyDown,t_key)[dbg_index]=false;
			}
		}
	}
	pop_err();
}
c_InputDevice.prototype.p_EndUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<207>";
	for(var t_i=0;t_i<this.m__keyHitPut;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<208>";
		dbg_array(this.m__keyHit,dbg_array(this.m__keyHitQueue,t_i)[dbg_index])[dbg_index]=0;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<210>";
	this.m__keyHitPut=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<211>";
	this.m__charGet=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<212>";
	this.m__charPut=0;
	pop_err();
}
c_InputDevice.prototype.p_KeyEvent=function(t_event,t_data){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<111>";
	var t_1=t_event;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<112>";
	if(t_1==1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<113>";
		if(!dbg_array(this.m__keyDown,t_data)[dbg_index]){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<114>";
			dbg_array(this.m__keyDown,t_data)[dbg_index]=true;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<115>";
			this.p_PutKeyHit(t_data);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<116>";
			if(t_data==1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<117>";
				dbg_array(this.m__keyDown,384)[dbg_index]=true;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<118>";
				this.p_PutKeyHit(384);
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<119>";
				if(t_data==384){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<120>";
					dbg_array(this.m__keyDown,1)[dbg_index]=true;
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<121>";
					this.p_PutKeyHit(1);
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<124>";
		if(t_1==2){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<125>";
			if(dbg_array(this.m__keyDown,t_data)[dbg_index]){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<126>";
				dbg_array(this.m__keyDown,t_data)[dbg_index]=false;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<127>";
				if(t_data==1){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<128>";
					dbg_array(this.m__keyDown,384)[dbg_index]=false;
				}else{
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<129>";
					if(t_data==384){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<130>";
						dbg_array(this.m__keyDown,1)[dbg_index]=false;
					}
				}
			}
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<133>";
			if(t_1==3){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<134>";
				if(this.m__charPut<this.m__charQueue.length){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<135>";
					dbg_array(this.m__charQueue,this.m__charPut)[dbg_index]=t_data;
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<136>";
					this.m__charPut+=1;
				}
			}
		}
	}
	pop_err();
}
c_InputDevice.prototype.p_MouseEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<142>";
	var t_2=t_event;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<143>";
	if(t_2==4){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<144>";
		this.p_KeyEvent(1,1+t_data);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<145>";
		if(t_2==5){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<146>";
			this.p_KeyEvent(2,1+t_data);
			pop_err();
			return;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<148>";
			if(t_2==6){
			}else{
				pop_err();
				return;
			}
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<152>";
	this.m__mouseX=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<153>";
	this.m__mouseY=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<154>";
	dbg_array(this.m__touchX,0)[dbg_index]=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<155>";
	dbg_array(this.m__touchY,0)[dbg_index]=t_y;
	pop_err();
}
c_InputDevice.prototype.p_TouchEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<159>";
	var t_3=t_event;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<160>";
	if(t_3==7){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<161>";
		this.p_KeyEvent(1,384+t_data);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<162>";
		if(t_3==8){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<163>";
			this.p_KeyEvent(2,384+t_data);
			pop_err();
			return;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<165>";
			if(t_3==9){
			}else{
				pop_err();
				return;
			}
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<169>";
	dbg_array(this.m__touchX,t_data)[dbg_index]=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<170>";
	dbg_array(this.m__touchY,t_data)[dbg_index]=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<171>";
	if(t_data==0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<172>";
		this.m__mouseX=t_x;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<173>";
		this.m__mouseY=t_y;
	}
	pop_err();
}
c_InputDevice.prototype.p_MotionEvent=function(t_event,t_data,t_x,t_y,t_z){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<178>";
	var t_4=t_event;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<179>";
	if(t_4==10){
	}else{
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<183>";
	this.m__accelX=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<184>";
	this.m__accelY=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<185>";
	this.m__accelZ=t_z;
	pop_err();
}
c_InputDevice.prototype.p_Reset=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<32>";
	for(var t_i=0;t_i<512;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<33>";
		dbg_array(this.m__keyDown,t_i)[dbg_index]=false;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<34>";
		dbg_array(this.m__keyHit,t_i)[dbg_index]=0;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<36>";
	this.m__keyHitPut=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<37>";
	this.m__charGet=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<38>";
	this.m__charPut=0;
	pop_err();
}
c_InputDevice.prototype.p_KeyDown=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<47>";
	if(t_key>0 && t_key<512){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<47>";
		pop_err();
		return dbg_array(this.m__keyDown,t_key)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<48>";
	pop_err();
	return false;
}
c_InputDevice.prototype.p_TouchX=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<77>";
	if(t_index>=0 && t_index<32){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<77>";
		pop_err();
		return dbg_array(this.m__touchX,t_index)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<78>";
	pop_err();
	return 0.0;
}
c_InputDevice.prototype.p_TouchY=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<82>";
	if(t_index>=0 && t_index<32){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<82>";
		pop_err();
		return dbg_array(this.m__touchY,t_index)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<83>";
	pop_err();
	return 0.0;
}
c_InputDevice.prototype.p_KeyHit=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<52>";
	if(t_key>0 && t_key<512){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<52>";
		pop_err();
		return dbg_array(this.m__keyHit,t_key)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<53>";
	pop_err();
	return 0;
}
function c_JoyState(){
	Object.call(this);
	this.m_joyx=new_number_array(2);
	this.m_joyy=new_number_array(2);
	this.m_joyz=new_number_array(2);
	this.m_buttons=new_bool_array(32);
}
c_JoyState.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/inputdevice.monkey<14>";
	pop_err();
	return this;
}
var bb_input_device=null;
function bb_input_SetInputDevice(t_dev){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/input.monkey<22>";
	bb_input_device=t_dev;
	pop_err();
	return 0;
}
var bb_app__devWidth=0;
var bb_app__devHeight=0;
function bb_app_ValidateDeviceWindow(t_notifyApp){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<57>";
	var t_w=bb_app__game.GetDeviceWidth();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<58>";
	var t_h=bb_app__game.GetDeviceHeight();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<59>";
	if(t_w==bb_app__devWidth && t_h==bb_app__devHeight){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<60>";
	bb_app__devWidth=t_w;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<61>";
	bb_app__devHeight=t_h;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<62>";
	if(t_notifyApp){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<62>";
		bb_app__app.p_OnResize();
	}
	pop_err();
}
function c_DisplayMode(){
	Object.call(this);
	this.m__width=0;
	this.m__height=0;
}
c_DisplayMode.m_new=function(t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<192>";
	this.m__width=t_width;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<193>";
	this.m__height=t_height;
	pop_err();
	return this;
}
c_DisplayMode.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<189>";
	pop_err();
	return this;
}
function c_Map(){
	Object.call(this);
	this.m_root=null;
}
c_Map.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<7>";
	pop_err();
	return this;
}
c_Map.prototype.p_Compare=function(t_lhs,t_rhs){
}
c_Map.prototype.p_FindNode=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<157>";
	var t_node=this.m_root;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<159>";
	while((t_node)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<160>";
		var t_cmp=this.p_Compare(t_key,dbg_object(t_node).m_key);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<161>";
		if(t_cmp>0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<162>";
			t_node=dbg_object(t_node).m_right;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<163>";
			if(t_cmp<0){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<164>";
				t_node=dbg_object(t_node).m_left;
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<166>";
				pop_err();
				return t_node;
			}
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<169>";
	pop_err();
	return t_node;
}
c_Map.prototype.p_Contains=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<25>";
	var t_=this.p_FindNode(t_key)!=null;
	pop_err();
	return t_;
}
c_Map.prototype.p_RotateLeft=function(t_node){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<251>";
	var t_child=dbg_object(t_node).m_right;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<252>";
	dbg_object(t_node).m_right=dbg_object(t_child).m_left;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<253>";
	if((dbg_object(t_child).m_left)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<254>";
		dbg_object(dbg_object(t_child).m_left).m_parent=t_node;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<256>";
	dbg_object(t_child).m_parent=dbg_object(t_node).m_parent;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<257>";
	if((dbg_object(t_node).m_parent)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<258>";
		if(t_node==dbg_object(dbg_object(t_node).m_parent).m_left){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<259>";
			dbg_object(dbg_object(t_node).m_parent).m_left=t_child;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<261>";
			dbg_object(dbg_object(t_node).m_parent).m_right=t_child;
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<264>";
		this.m_root=t_child;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<266>";
	dbg_object(t_child).m_left=t_node;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<267>";
	dbg_object(t_node).m_parent=t_child;
	pop_err();
	return 0;
}
c_Map.prototype.p_RotateRight=function(t_node){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<271>";
	var t_child=dbg_object(t_node).m_left;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<272>";
	dbg_object(t_node).m_left=dbg_object(t_child).m_right;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<273>";
	if((dbg_object(t_child).m_right)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<274>";
		dbg_object(dbg_object(t_child).m_right).m_parent=t_node;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<276>";
	dbg_object(t_child).m_parent=dbg_object(t_node).m_parent;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<277>";
	if((dbg_object(t_node).m_parent)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<278>";
		if(t_node==dbg_object(dbg_object(t_node).m_parent).m_right){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<279>";
			dbg_object(dbg_object(t_node).m_parent).m_right=t_child;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<281>";
			dbg_object(dbg_object(t_node).m_parent).m_left=t_child;
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<284>";
		this.m_root=t_child;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<286>";
	dbg_object(t_child).m_right=t_node;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<287>";
	dbg_object(t_node).m_parent=t_child;
	pop_err();
	return 0;
}
c_Map.prototype.p_InsertFixup=function(t_node){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<212>";
	while(((dbg_object(t_node).m_parent)!=null) && dbg_object(dbg_object(t_node).m_parent).m_color==-1 && ((dbg_object(dbg_object(t_node).m_parent).m_parent)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<213>";
		if(dbg_object(t_node).m_parent==dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_left){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<214>";
			var t_uncle=dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_right;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<215>";
			if(((t_uncle)!=null) && dbg_object(t_uncle).m_color==-1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<216>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<217>";
				dbg_object(t_uncle).m_color=1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<218>";
				dbg_object(dbg_object(t_uncle).m_parent).m_color=-1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<219>";
				t_node=dbg_object(t_uncle).m_parent;
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<221>";
				if(t_node==dbg_object(dbg_object(t_node).m_parent).m_right){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<222>";
					t_node=dbg_object(t_node).m_parent;
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<223>";
					this.p_RotateLeft(t_node);
				}
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<225>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<226>";
				dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_color=-1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<227>";
				this.p_RotateRight(dbg_object(dbg_object(t_node).m_parent).m_parent);
			}
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<230>";
			var t_uncle2=dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_left;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<231>";
			if(((t_uncle2)!=null) && dbg_object(t_uncle2).m_color==-1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<232>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<233>";
				dbg_object(t_uncle2).m_color=1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<234>";
				dbg_object(dbg_object(t_uncle2).m_parent).m_color=-1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<235>";
				t_node=dbg_object(t_uncle2).m_parent;
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<237>";
				if(t_node==dbg_object(dbg_object(t_node).m_parent).m_left){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<238>";
					t_node=dbg_object(t_node).m_parent;
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<239>";
					this.p_RotateRight(t_node);
				}
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<241>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<242>";
				dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_color=-1;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<243>";
				this.p_RotateLeft(dbg_object(dbg_object(t_node).m_parent).m_parent);
			}
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<247>";
	dbg_object(this.m_root).m_color=1;
	pop_err();
	return 0;
}
c_Map.prototype.p_Set=function(t_key,t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<29>";
	var t_node=this.m_root;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<30>";
	var t_parent=null;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<30>";
	var t_cmp=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<32>";
	while((t_node)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<33>";
		t_parent=t_node;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<34>";
		t_cmp=this.p_Compare(t_key,dbg_object(t_node).m_key);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<35>";
		if(t_cmp>0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<36>";
			t_node=dbg_object(t_node).m_right;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<37>";
			if(t_cmp<0){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<38>";
				t_node=dbg_object(t_node).m_left;
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<40>";
				dbg_object(t_node).m_value=t_value;
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<41>";
				pop_err();
				return false;
			}
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<45>";
	t_node=c_Node.m_new.call(new c_Node,t_key,t_value,-1,t_parent);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<47>";
	if((t_parent)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<48>";
		if(t_cmp>0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<49>";
			dbg_object(t_parent).m_right=t_node;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<51>";
			dbg_object(t_parent).m_left=t_node;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<53>";
		this.p_InsertFixup(t_node);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<55>";
		this.m_root=t_node;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<57>";
	pop_err();
	return true;
}
c_Map.prototype.p_Insert=function(t_key,t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<146>";
	var t_=this.p_Set(t_key,t_value);
	pop_err();
	return t_;
}
function c_IntMap(){
	c_Map.call(this);
}
c_IntMap.prototype=extend_class(c_Map);
c_IntMap.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<534>";
	c_Map.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<534>";
	pop_err();
	return this;
}
c_IntMap.prototype.p_Compare=function(t_lhs,t_rhs){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<537>";
	var t_=t_lhs-t_rhs;
	pop_err();
	return t_;
}
function c_Stack(){
	Object.call(this);
	this.m_data=[];
	this.m_length=0;
}
c_Stack.m_new=function(){
	push_err();
	pop_err();
	return this;
}
c_Stack.m_new2=function(t_data){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<13>";
	dbg_object(this).m_data=t_data.slice(0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<14>";
	dbg_object(this).m_length=t_data.length;
	pop_err();
	return this;
}
c_Stack.prototype.p_Push=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<67>";
	if(this.m_length==this.m_data.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<68>";
		this.m_data=resize_object_array(this.m_data,this.m_length*2+10);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<70>";
	dbg_array(this.m_data,this.m_length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<71>";
	this.m_length+=1;
	pop_err();
}
c_Stack.prototype.p_Push2=function(t_values,t_offset,t_count){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<79>";
	for(var t_i=0;t_i<t_count;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<80>";
		this.p_Push(dbg_array(t_values,t_offset+t_i)[dbg_index]);
	}
	pop_err();
}
c_Stack.prototype.p_Push3=function(t_values,t_offset){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<75>";
	this.p_Push2(t_values,t_offset,t_values.length-t_offset);
	pop_err();
}
c_Stack.prototype.p_ToArray=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<18>";
	var t_t=new_object_array(this.m_length);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<19>";
	for(var t_i=0;t_i<this.m_length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<20>";
		dbg_array(t_t,t_i)[dbg_index]=dbg_array(this.m_data,t_i)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/stack.monkey<22>";
	pop_err();
	return t_t;
}
function c_Node(){
	Object.call(this);
	this.m_key=0;
	this.m_right=null;
	this.m_left=null;
	this.m_value=null;
	this.m_color=0;
	this.m_parent=null;
}
c_Node.m_new=function(t_key,t_value,t_color,t_parent){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<364>";
	dbg_object(this).m_key=t_key;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<365>";
	dbg_object(this).m_value=t_value;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<366>";
	dbg_object(this).m_color=t_color;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<367>";
	dbg_object(this).m_parent=t_parent;
	pop_err();
	return this;
}
c_Node.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/map.monkey<361>";
	pop_err();
	return this;
}
var bb_app__displayModes=[];
var bb_app__desktopMode=null;
function bb_app_DeviceWidth(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<263>";
	pop_err();
	return bb_app__devWidth;
}
function bb_app_DeviceHeight(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<267>";
	pop_err();
	return bb_app__devHeight;
}
function bb_app_EnumDisplayModes(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<33>";
	var t_modes=bb_app__game.GetDisplayModes();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<34>";
	var t_mmap=c_IntMap.m_new.call(new c_IntMap);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<35>";
	var t_mstack=c_Stack.m_new.call(new c_Stack);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<36>";
	for(var t_i=0;t_i<t_modes.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<37>";
		var t_w=dbg_object(dbg_array(t_modes,t_i)[dbg_index]).width;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<38>";
		var t_h=dbg_object(dbg_array(t_modes,t_i)[dbg_index]).height;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<39>";
		var t_size=t_w<<16|t_h;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<40>";
		if(t_mmap.p_Contains(t_size)){
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<42>";
			var t_mode=c_DisplayMode.m_new.call(new c_DisplayMode,dbg_object(dbg_array(t_modes,t_i)[dbg_index]).width,dbg_object(dbg_array(t_modes,t_i)[dbg_index]).height);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<43>";
			t_mmap.p_Insert(t_size,t_mode);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<44>";
			t_mstack.p_Push(t_mode);
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<47>";
	bb_app__displayModes=t_mstack.p_ToArray();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<48>";
	var t_mode2=bb_app__game.GetDesktopMode();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<49>";
	if((t_mode2)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<50>";
		bb_app__desktopMode=c_DisplayMode.m_new.call(new c_DisplayMode,dbg_object(t_mode2).width,dbg_object(t_mode2).height);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<52>";
		bb_app__desktopMode=c_DisplayMode.m_new.call(new c_DisplayMode,bb_app_DeviceWidth(),bb_app_DeviceHeight());
	}
	pop_err();
}
var bb_graphics_renderDevice=null;
function bb_graphics_SetMatrix(t_ix,t_iy,t_jx,t_jy,t_tx,t_ty){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<312>";
	dbg_object(bb_graphics_context).m_ix=t_ix;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<313>";
	dbg_object(bb_graphics_context).m_iy=t_iy;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<314>";
	dbg_object(bb_graphics_context).m_jx=t_jx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<315>";
	dbg_object(bb_graphics_context).m_jy=t_jy;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<316>";
	dbg_object(bb_graphics_context).m_tx=t_tx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<317>";
	dbg_object(bb_graphics_context).m_ty=t_ty;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<318>";
	dbg_object(bb_graphics_context).m_tformed=((t_ix!=1.0 || t_iy!=0.0 || t_jx!=0.0 || t_jy!=1.0 || t_tx!=0.0 || t_ty!=0.0)?1:0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<319>";
	dbg_object(bb_graphics_context).m_matDirty=1;
	pop_err();
	return 0;
}
function bb_graphics_SetMatrix2(t_m){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<308>";
	bb_graphics_SetMatrix(dbg_array(t_m,0)[dbg_index],dbg_array(t_m,1)[dbg_index],dbg_array(t_m,2)[dbg_index],dbg_array(t_m,3)[dbg_index],dbg_array(t_m,4)[dbg_index],dbg_array(t_m,5)[dbg_index]);
	pop_err();
	return 0;
}
function bb_graphics_SetColor(t_r,t_g,t_b){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<254>";
	dbg_object(bb_graphics_context).m_color_r=t_r;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<255>";
	dbg_object(bb_graphics_context).m_color_g=t_g;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<256>";
	dbg_object(bb_graphics_context).m_color_b=t_b;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<257>";
	bb_graphics_renderDevice.SetColor(t_r,t_g,t_b);
	pop_err();
	return 0;
}
function bb_graphics_SetAlpha(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<271>";
	dbg_object(bb_graphics_context).m_alpha=t_alpha;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<272>";
	bb_graphics_renderDevice.SetAlpha(t_alpha);
	pop_err();
	return 0;
}
function bb_graphics_SetBlend(t_blend){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<280>";
	dbg_object(bb_graphics_context).m_blend=t_blend;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<281>";
	bb_graphics_renderDevice.SetBlend(t_blend);
	pop_err();
	return 0;
}
function bb_graphics_SetScissor(t_x,t_y,t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<289>";
	dbg_object(bb_graphics_context).m_scissor_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<290>";
	dbg_object(bb_graphics_context).m_scissor_y=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<291>";
	dbg_object(bb_graphics_context).m_scissor_width=t_width;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<292>";
	dbg_object(bb_graphics_context).m_scissor_height=t_height;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<293>";
	bb_graphics_renderDevice.SetScissor(((t_x)|0),((t_y)|0),((t_width)|0),((t_height)|0));
	pop_err();
	return 0;
}
function bb_graphics_BeginRender(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<225>";
	bb_graphics_renderDevice=bb_graphics_device;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<226>";
	dbg_object(bb_graphics_context).m_matrixSp=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<227>";
	bb_graphics_SetMatrix(1.0,0.0,0.0,1.0,0.0,0.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<228>";
	bb_graphics_SetColor(255.0,255.0,255.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<229>";
	bb_graphics_SetAlpha(1.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<230>";
	bb_graphics_SetBlend(0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<231>";
	bb_graphics_SetScissor(0.0,0.0,(bb_app_DeviceWidth()),(bb_app_DeviceHeight()));
	pop_err();
	return 0;
}
function bb_graphics_EndRender(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<235>";
	bb_graphics_renderDevice=null;
	pop_err();
	return 0;
}
function c_BBGameEvent(){
	Object.call(this);
}
function bb_app_EndApp(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<259>";
	error("");
	pop_err();
}
function c_iObject(){
	Object.call(this);
	this.m_enabled=1;
	this.m_tattoo=false;
	this.m_inPool=0;
}
c_iObject.prototype.p_OnDisabled=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/pooling.monkey<133>";
	pop_err();
	return 0;
}
c_iObject.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/pooling.monkey<20>";
	pop_err();
	return this;
}
function c_iScene(){
	c_iObject.call(this);
	this.m_visible=1;
	this.m_started=false;
	this.m_cold=true;
	this.m_paused=false;
}
c_iScene.prototype=extend_class(c_iObject);
c_iScene.prototype.p_OnBack=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<371>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnClose=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<384>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnLoading=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<409>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<441>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<509>";
	this.p_OnRender();
	pop_err();
}
c_iScene.prototype.p_OnResize=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<452>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Resume=function(){
	push_err();
	pop_err();
}
c_iScene.prototype.p_OnResume=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<458>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Suspend=function(){
	push_err();
	pop_err();
}
c_iScene.prototype.p_OnSuspend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<483>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnStop=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<472>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Set2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<538>";
	bb_app2_iCurrentScene=this;
	pop_err();
}
c_iScene.prototype.p_OnColdStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<393>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<465>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnPaused=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<425>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<490>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<567>";
	if((bb_app2_iNextScene)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<569>";
		if(this.m_started){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<570>";
			bb_input_ResetInput();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<571>";
			this.m_started=false;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<572>";
			this.p_OnStop();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<575>";
		bb_app2_iNextScene.p_Set2();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<577>";
		bb_app2_iNextScene=null;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<579>";
		if(dbg_object(bb_app2_iCurrentScene).m_cold==true){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<580>";
			bb_app2_iCurrentScene.p_OnColdStart();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<581>";
			dbg_object(bb_app2_iCurrentScene).m_cold=false;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<584>";
		if(dbg_object(bb_app2_iCurrentScene).m_started==false){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<585>";
			bb_app2_iCurrentScene.p_OnStart();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<586>";
			dbg_object(bb_app2_iCurrentScene).m_started=true;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<589>";
		if(dbg_object(bb_app2_iCurrentScene).m_enabled==1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<591>";
			if(dbg_object(bb_app2_iCurrentScene).m_paused){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<592>";
				bb_app2_iCurrentScene.p_OnPaused();
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<594>";
				bb_app2_iCurrentScene.p_OnUpdate();
			}
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<599>";
			if(dbg_object(bb_app2_iCurrentScene).m_enabled>1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<600>";
				bb_app2_iCurrentScene.p_OnDisabled();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<601>";
				dbg_object(bb_app2_iCurrentScene).m_enabled=dbg_object(bb_app2_iCurrentScene).m_enabled-1;
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<608>";
		if(dbg_object(bb_app2_iCurrentScene).m_cold==true){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<609>";
			bb_app2_iCurrentScene.p_OnColdStart();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<610>";
			dbg_object(bb_app2_iCurrentScene).m_cold=false;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<613>";
		if(dbg_object(bb_app2_iCurrentScene).m_started==false){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<614>";
			bb_app2_iCurrentScene.p_OnStart();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<615>";
			dbg_object(bb_app2_iCurrentScene).m_started=true;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<618>";
		if(dbg_object(bb_app2_iCurrentScene).m_enabled==1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<620>";
			if(dbg_object(bb_app2_iCurrentScene).m_paused){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<621>";
				bb_app2_iCurrentScene.p_OnPaused();
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<623>";
				bb_app2_iCurrentScene.p_OnUpdate();
			}
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<628>";
			if(dbg_object(bb_app2_iCurrentScene).m_enabled>1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<629>";
				bb_app2_iCurrentScene.p_OnDisabled();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<630>";
				dbg_object(bb_app2_iCurrentScene).m_enabled=dbg_object(bb_app2_iCurrentScene).m_enabled-1;
			}
		}
	}
	pop_err();
}
c_iScene.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<402>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_SystemInit=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<556>";
	this.p_Set2();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<558>";
	this.p_OnCreate();
	pop_err();
}
c_iScene.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<357>";
	c_iObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<358>";
	this.p_SystemInit();
	pop_err();
	return this;
}
var bb_app2_iCurrentScene=null;
function bb_app2_iBack(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<41>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m_enabled)!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<41>";
		bb_app2_iCurrentScene.p_OnBack();
	}
	pop_err();
}
function bb_app2_iClose(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<59>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m_enabled)!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<59>";
		bb_app2_iCurrentScene.p_OnClose();
	}
	pop_err();
}
function bb_app2_iLoading(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<94>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m_enabled)!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<94>";
		bb_app2_iCurrentScene.p_OnLoading();
	}
	pop_err();
}
function bb_app2_iRender(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<138>";
	if((bb_app2_iCurrentScene)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<140>";
		if((dbg_object(bb_app2_iCurrentScene).m_enabled)!=0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<142>";
			if(dbg_object(bb_app2_iCurrentScene).m_visible==1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<143>";
				bb_app2_iCurrentScene.p_Render();
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<145>";
				if(dbg_object(bb_app2_iCurrentScene).m_visible>1){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<145>";
					dbg_object(bb_app2_iCurrentScene).m_visible=dbg_object(bb_app2_iCurrentScene).m_visible-1;
				}
			}
		}
	}
	pop_err();
}
function bb_app2_iResize(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<180>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m_enabled)!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<180>";
		bb_app2_iCurrentScene.p_OnResize();
	}
	pop_err();
}
function bb_app2_iResume(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<198>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m_enabled)!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<199>";
		bb_app2_iCurrentScene.p_Resume();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<200>";
		bb_app2_iCurrentScene.p_OnResume();
	}
	pop_err();
}
function bb_app2_iSuspend(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<237>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m_enabled)!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<238>";
		bb_app2_iCurrentScene.p_Suspend();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<239>";
		bb_app2_iCurrentScene.p_OnSuspend();
	}
	pop_err();
}
function c_iDeltaTimer(){
	Object.call(this);
	this.m_targetFPS=60.0;
	this.m_lastTicks=.0;
	this.m_currentTicks=.0;
	this.m_frameTime=.0;
	this.m_elapsedTime=0.0;
	this.m_timeScale=1.0;
	this.m_deltaTime=.0;
	this.m_elapsedDelta=0.0;
}
c_iDeltaTimer.m_new=function(t_targetFPS){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<25>";
	dbg_object(this).m_targetFPS=t_targetFPS;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<26>";
	dbg_object(this).m_lastTicks=(bb_app_Millisecs());
	pop_err();
	return this;
}
c_iDeltaTimer.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<11>";
	pop_err();
	return this;
}
c_iDeltaTimer.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<100>";
	dbg_object(this).m_currentTicks=(bb_app_Millisecs());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<101>";
	dbg_object(this).m_frameTime=dbg_object(this).m_currentTicks-dbg_object(this).m_lastTicks;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<102>";
	dbg_object(this).m_elapsedTime+=dbg_object(this).m_frameTime;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<103>";
	dbg_object(this).m_deltaTime=dbg_object(this).m_frameTime/(1000.0/dbg_object(this).m_targetFPS)*dbg_object(this).m_timeScale;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<104>";
	dbg_object(this).m_elapsedDelta=dbg_object(this).m_elapsedDelta+dbg_object(this).m_deltaTime/dbg_object(this).m_targetFPS;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<105>";
	dbg_object(this).m_lastTicks=dbg_object(this).m_currentTicks;
	pop_err();
}
c_iDeltaTimer.prototype.p_Resume=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/deltatimer.monkey<63>";
	dbg_object(this).m_lastTicks=(bb_app_Millisecs());
	pop_err();
}
function bb_app_Millisecs(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<233>";
	var t_=bb_app__game.Millisecs();
	pop_err();
	return t_;
}
var bb_app2_iDT=null;
var bb_app2_iNextScene=null;
function bb_input_ResetInput(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/input.monkey<26>";
	bb_input_device.p_Reset();
	pop_err();
	return 0;
}
function bb_app2_iUpdate(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<259>";
	bb_app2_iDT.p_Update();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<261>";
	if((bb_app2_iCurrentScene)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<262>";
		bb_app2_iCurrentScene.p_Update();
	}
	pop_err();
}
var bb_random_Seed=0;
function bb_gfx_iLoadSprite(t_path,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<384>";
	var t_=bb_graphics_LoadImage(t_path,t_frameCount,1);
	pop_err();
	return t_;
}
function bb_gfx_iLoadSprite2(t_path,t_frameWidth,t_frameHeight,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<393>";
	var t_=bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,1);
	pop_err();
	return t_;
}
function bb_strings_iStripExt(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/strings.monkey<245>";
	var t_i=t_path.lastIndexOf(".");
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/strings.monkey<246>";
	if(t_i!=-1 && t_path.indexOf("/",t_i+1)==-1 && t_path.indexOf("\\",t_i+1)==-1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/strings.monkey<246>";
		var t_=t_path.slice(0,t_i);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/strings.monkey<248>";
	pop_err();
	return t_path;
}
function bb_strings_iExtractExt(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/strings.monkey<303>";
	var t_i=t_path.lastIndexOf(".");
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/strings.monkey<304>";
	if(t_i!=-1 && t_path.indexOf("/",t_i+1)==-1 && t_path.indexOf("\\",t_i+1)==-1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/strings.monkey<304>";
		var t_=t_path.slice(t_i+1);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/strings.monkey<305>";
	pop_err();
	return "";
}
function bb_gfx_iLoadImage(t_start,t_count,t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<318>";
	var t_i=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<320>";
	var t_image=new_object_array(t_count);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<322>";
	var t_file=bb_strings_iStripExt(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<323>";
	var t_extension="."+bb_strings_iExtractExt(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<325>";
	for(var t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<326>";
		if(t_c<10){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<326>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"000"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<327>";
		if(t_c>9 && t_c<100){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<327>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"00"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<328>";
		if(t_c>99 && t_c<1000){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<328>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"0"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<329>";
		t_i=t_i+1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<332>";
	pop_err();
	return t_image;
}
function bb_gfx_iLoadImage2(t_image,t_start,t_count,t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<338>";
	var t_i=t_image.length;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<340>";
	t_image=resize_object_array(t_image,t_image.length+t_count);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<342>";
	var t_file=bb_strings_iStripExt(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<343>";
	var t_extension="."+bb_strings_iExtractExt(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<345>";
	for(var t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<346>";
		if(t_c<10){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<346>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"000"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<347>";
		if(t_c>9 && t_c<100){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<347>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"00"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<348>";
		if(t_c>99 && t_c<1000){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<348>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"0"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<349>";
		t_i=t_i+1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<352>";
	pop_err();
	return t_image;
}
function bb_gfx_iLoadImage3(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<363>";
	var t_i=new_object_array(t_count);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<365>";
	var t_f=bb_strings_iStripExt(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<366>";
	var t_e="."+bb_strings_iExtractExt(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<368>";
	for(var t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<369>";
		if(t_c<10){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<369>";
			dbg_array(t_i,t_c)[dbg_index]=bb_graphics_LoadImage2(t_f+"000"+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<370>";
		if(t_c>9 && t_c<100){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<370>";
			dbg_array(t_i,t_c)[dbg_index]=bb_graphics_LoadImage2(t_f+"00"+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<371>";
		if(t_c>99 && t_c<1000){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<371>";
			dbg_array(t_i,t_c)[dbg_index]=bb_graphics_LoadImage2(t_f+"0"+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<374>";
	pop_err();
	return t_i;
}
function bb_gfx_iLoadSprite3(t_start,t_count,t_path,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<402>";
	var t_=bb_gfx_iLoadImage(t_start,t_count,t_path,t_frameCount,1);
	pop_err();
	return t_;
}
function bb_gfx_iLoadSprite4(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<411>";
	var t_=bb_gfx_iLoadImage3(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,1);
	pop_err();
	return t_;
}
var bb_main_gfxMonkey=null;
function c_iEngine(){
	c_iScene.call(this);
	this.m_timeCode=0;
	this.m_playfieldList=c_iList.m_new.call(new c_iList);
	this.m_autoCls=true;
	this.m_clsRed=.0;
	this.m_clsGreen=.0;
	this.m_clsBlue=.0;
	this.m_borders=true;
	this.m_bordersRed=.0;
	this.m_bordersGreen=.0;
	this.m_bordersBlue=.0;
	this.m_colorFade=1.0;
	this.m_renderToPlayfield=true;
	this.m_currentPlayfield=null;
	this.m_alphaFade=1.0;
	this.m_showSystemGui=false;
	this.m_taskList=c_iList5.m_new.call(new c_iList5);
	this.m_debugRender=true;
	this.m_countDown=0;
	this.m_scoreCount=0;
	this.m_playfieldPointer=null;
	this.m_collisionReadList=c_iList3.m_new.call(new c_iList3);
	this.m_layerPointer=null;
	this.m_collisionWriteList=c_iList3.m_new.call(new c_iList3);
}
c_iEngine.prototype=extend_class(c_iScene);
c_iEngine.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<13>";
	c_iScene.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<13>";
	pop_err();
	return this;
}
c_iEngine.prototype.p_OnColdStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<400>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_OnResize=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<409>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_Playfield=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<455>";
	var t_p=dbg_object(this).m_playfieldList.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<457>";
	while((t_p)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<458>";
		if(dbg_object(t_p).m_enabled==1 && dbg_object(t_p).m_visible){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<458>";
			pop_err();
			return t_p;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<459>";
		t_p=this.m_playfieldList.p_Ascend();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<462>";
	pop_err();
	return null;
}
c_iEngine.prototype.p_AutoCls=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<169>";
	var t_p=this.p_Playfield();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<171>";
	if((t_p)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<173>";
		if(!dbg_object(t_p).m_autoCls && dbg_object(this).m_autoCls){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<173>";
			bb_gfx_iCls(dbg_object(this).m_clsRed,dbg_object(this).m_clsGreen,dbg_object(this).m_clsBlue);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<175>";
		if(dbg_object(this).m_borders){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<175>";
			t_p.p_ClearBorders(((dbg_object(this).m_bordersRed)|0),((dbg_object(this).m_bordersGreen)|0),((dbg_object(this).m_bordersBlue)|0));
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<179>";
		if(dbg_object(this).m_autoCls){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<179>";
			bb_gfx_iCls(dbg_object(this).m_clsRed*dbg_object(this).m_colorFade,dbg_object(this).m_clsGreen*dbg_object(this).m_colorFade,dbg_object(this).m_clsBlue*dbg_object(this).m_colorFade);
		}
	}
	pop_err();
}
c_iEngine.prototype.p_AutoCls2=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<190>";
	dbg_object(this).m_clsRed=(t_red);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<191>";
	dbg_object(this).m_clsGreen=(t_green);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<192>";
	dbg_object(this).m_clsBlue=(t_blue);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<194>";
	dbg_object(this).m_autoCls=true;
	pop_err();
}
c_iEngine.prototype.p_AutoCls3=function(t_state){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<202>";
	dbg_object(this).m_autoCls=t_state;
	pop_err();
}
c_iEngine.prototype.p_AlphaFade=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<160>";
	pop_err();
	return dbg_object(this).m_alphaFade;
}
c_iEngine.prototype.p_AlphaFade2=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<164>";
	dbg_object(this).m_alphaFade=t_alpha;
	pop_err();
}
c_iEngine.prototype.p_SetAlpha=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<614>";
	if((dbg_object(this).m_currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<615>";
		dbg_object(this).m_currentPlayfield.p_SetAlpha(t_alpha);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<617>";
		bb_graphics_SetAlpha(t_alpha*bb_globals_iEnginePointer.p_AlphaFade());
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<620>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_ColorFade=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<233>";
	pop_err();
	return dbg_object(this).m_colorFade;
}
c_iEngine.prototype.p_ColorFade2=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<237>";
	dbg_object(this).m_colorFade=t_value;
	pop_err();
}
c_iEngine.prototype.p_SetColor=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<629>";
	if((dbg_object(this).m_currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<630>";
		dbg_object(this).m_currentPlayfield.p_SetColor(t_red,t_green,t_blue);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<632>";
		bb_graphics_SetColor(t_red*bb_globals_iEnginePointer.p_ColorFade(),t_green*bb_globals_iEnginePointer.p_ColorFade(),t_blue*bb_globals_iEnginePointer.p_ColorFade());
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<635>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_OnTopRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<418>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_RenderPlayfields=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<558>";
	dbg_object(this).m_currentPlayfield=dbg_object(this).m_playfieldList.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<560>";
	while((dbg_object(this).m_currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<562>";
		if(dbg_object(dbg_object(this).m_currentPlayfield).m_enabled==1 && dbg_object(dbg_object(this).m_currentPlayfield).m_visible){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<562>";
			dbg_object(this).m_currentPlayfield.p_Render();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<564>";
		dbg_object(this).m_currentPlayfield=dbg_object(this).m_playfieldList.p_Ascend();
	}
	pop_err();
}
c_iEngine.prototype.p_RenderSystemGui=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<575>";
	if(dbg_object(this).m_showSystemGui && ((c_iSystemGui.m_playfield)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<577>";
		c_iStack2D.m_Push();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<579>";
		bb_graphics_Scale(dbg_object(c_iSystemGui.m_playfield).m_scaleX,dbg_object(c_iSystemGui.m_playfield).m_scaleY);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<581>";
		bb_graphics_Translate(c_iSystemGui.m_playfield.p_PositionX()/dbg_object(c_iSystemGui.m_playfield).m_scaleX,c_iSystemGui.m_playfield.p_PositionY()/dbg_object(c_iSystemGui.m_playfield).m_scaleX);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<583>";
		var t_g=dbg_array(dbg_object(c_iSystemGui.m_playfield).m_guiList,dbg_object(c_iSystemGui.m_playfield).m_guiPage)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<584>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<585>";
			if(dbg_object(t_g).m_enabled==1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<586>";
				if(dbg_object(t_g).m_ghost){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<586>";
					t_g.p_RenderGhost();
				}else{
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<586>";
					t_g.p_Render();
				}
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<587>";
				t_g=dbg_array(dbg_object(c_iSystemGui.m_playfield).m_guiList,dbg_object(c_iSystemGui.m_playfield).m_guiPage)[dbg_index].p_Ascend();
			}
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<591>";
		c_iStack2D.m_Pop();
	}
	pop_err();
}
c_iEngine.prototype.p_DebugRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<254>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<492>";
	if((bb_functions_iTimeCode())!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<494>";
		this.p_AutoCls();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<496>";
		if(dbg_object(this).m_renderToPlayfield==false || dbg_object(this).m_playfieldList.p_Length()==0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<498>";
			c_iStack2D.m_Push();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<500>";
			this.p_SetAlpha(1.0);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<501>";
			this.p_SetColor(255.0,255.0,255.0);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<503>";
			this.p_OnRender();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<505>";
			c_iStack2D.m_Pop();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<509>";
		this.p_RenderPlayfields();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<511>";
		if(dbg_object(this).m_renderToPlayfield==false || dbg_object(this).m_playfieldList.p_Length()==0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<513>";
			c_iStack2D.m_Push();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<515>";
			this.p_SetAlpha(1.0);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<516>";
			this.p_SetColor(255.0,255.0,255.0);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<518>";
			this.p_OnTopRender();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<520>";
			c_iStack2D.m_Pop();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<524>";
		this.p_RenderSystemGui();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<526>";
		c_iTask.m_Render(this.m_taskList);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<528>";
		c_iTask.m_Render(bb_globals_iTaskList);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<530>";
		if(dbg_object(this).m_debugRender){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<532>";
			c_iStack2D.m_Push();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<534>";
			var t_a=this.m_alphaFade;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<535>";
			var t_c=this.m_colorFade;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<537>";
			this.m_alphaFade=1.0;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<538>";
			this.m_colorFade=1.0;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<540>";
			this.p_DebugRender();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<542>";
			this.m_alphaFade=t_a;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<543>";
			this.m_colorFade=t_c;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<545>";
			c_iStack2D.m_Pop();
		}
	}
	pop_err();
}
c_iEngine.prototype.p_Resume=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<598>";
	bb_app2_iDT.p_Resume();
	pop_err();
}
c_iEngine.prototype.p_Set2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<605>";
	bb_app2_iCurrentScene=(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<606>";
	bb_globals_iEnginePointer=this;
	pop_err();
}
c_iEngine.prototype.p_UpdateSystemGui=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<832>";
	if(dbg_object(this).m_showSystemGui && ((c_iSystemGui.m_playfield)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<834>";
		if(dbg_object(c_iSystemGui.m_playfield).m_nextGuiPage>=0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<835>";
			dbg_object(c_iSystemGui.m_playfield).m_guiPage=dbg_object(c_iSystemGui.m_playfield).m_nextGuiPage;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<836>";
			dbg_object(c_iSystemGui.m_playfield).m_nextGuiPage=-1;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<839>";
		var t_g=null;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<841>";
		t_g=dbg_array(dbg_object(c_iSystemGui.m_playfield).m_guiList,dbg_object(c_iSystemGui.m_playfield).m_guiPage)[dbg_index].p_Last();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<842>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<844>";
			t_g.p_UpdateWorldXY();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<846>";
			if(dbg_object(t_g).m_enabled==1 && !dbg_object(t_g).m_ghost){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<846>";
				t_g.p_UpdateInput();
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<848>";
			t_g=dbg_array(dbg_object(c_iSystemGui.m_playfield).m_guiList,dbg_object(c_iSystemGui.m_playfield).m_guiPage)[dbg_index].p_Descend();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<852>";
		t_g=dbg_array(dbg_object(c_iSystemGui.m_playfield).m_guiList,dbg_object(c_iSystemGui.m_playfield).m_guiPage)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<853>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<855>";
			if(dbg_object(t_g).m_enabled==1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<857>";
				if(dbg_object(t_g).m_ghost){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<858>";
					t_g.p_UpdateGhost();
				}else{
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<860>";
					t_g.p_Update();
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<861>";
					t_g.p_OnUpdate();
				}
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<866>";
			t_g=dbg_array(dbg_object(c_iSystemGui.m_playfield).m_guiList,dbg_object(c_iSystemGui.m_playfield).m_guiPage)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iEngine.prototype.p_UpdatePlayfields=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<784>";
	dbg_object(this).m_currentPlayfield=dbg_object(this).m_playfieldList.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<786>";
	while((dbg_object(this).m_currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<787>";
		if((dbg_object(dbg_object(this).m_currentPlayfield).m_enabled)!=0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<787>";
			dbg_object(this).m_currentPlayfield.p_Update();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<788>";
		dbg_object(this).m_currentPlayfield=dbg_object(this).m_playfieldList.p_Ascend();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<791>";
	dbg_object(this).m_currentPlayfield=dbg_object(this).m_playfieldList.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<793>";
	while((dbg_object(this).m_currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<795>";
		if((dbg_object(dbg_object(this).m_currentPlayfield).m_enabled)!=0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<797>";
			dbg_object(dbg_object(this).m_currentPlayfield).m_currentLayer=dbg_object(dbg_object(this).m_currentPlayfield).m_layerList.p_First();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<799>";
			while((dbg_object(dbg_object(this).m_currentPlayfield).m_currentLayer)!=null){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<801>";
				if(((dbg_object(dbg_object(dbg_object(this).m_currentPlayfield).m_currentLayer).m_enabled)!=0) && dbg_object(dbg_object(dbg_object(this).m_currentPlayfield).m_currentLayer).m_timeStamp!=dbg_object(this).m_timeCode){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<803>";
					dbg_object(dbg_object(dbg_object(this).m_currentPlayfield).m_currentLayer).m_timeStamp=dbg_object(this).m_timeCode;
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<805>";
					dbg_object(dbg_object(this).m_currentPlayfield).m_currentLayer.p_Update();
				}
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<809>";
				dbg_object(dbg_object(this).m_currentPlayfield).m_currentLayer=dbg_object(dbg_object(this).m_currentPlayfield).m_layerList.p_Ascend();
			}
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<815>";
		dbg_object(this).m_currentPlayfield=dbg_object(this).m_playfieldList.p_Ascend();
	}
	pop_err();
}
c_iEngine.prototype.p_UpdateService=function(){
	push_err();
	pop_err();
}
c_iEngine.prototype.p_UpdateCollisions=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<720>";
	var t_s1=dbg_object(this).m_collisionReadList.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<721>";
	while((t_s1)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<723>";
		if(((t_s1.p_Layer())!=null) && dbg_object(t_s1.p_Layer()).m_enabled==1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<725>";
			var t_s2=dbg_object(this).m_collisionWriteList.p_First();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<726>";
			while((t_s2)!=null){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<728>";
				if(((t_s2.p_Layer())!=null) && dbg_object(t_s2.p_Layer()).m_enabled==1){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<729>";
					if((t_s1.p_Collides(t_s2))!=0){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<729>";
						t_s1.p_OnCollision(t_s2);
					}
				}
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<732>";
				t_s2=dbg_object(this).m_collisionWriteList.p_Ascend();
			}
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<738>";
		t_s1=dbg_object(this).m_collisionReadList.p_Ascend();
	}
	pop_err();
}
c_iEngine.prototype.p_UpdateLogic=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<749>";
	if(dbg_object(this).m_enabled==1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<751>";
		this.p_UpdatePlayfields();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<753>";
		this.p_UpdateSystemGui();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<755>";
		this.p_UpdateService();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<757>";
		this.p_OnUpdate();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<759>";
		this.p_UpdateCollisions();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<761>";
		c_iTask.m_Update(this.m_taskList);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<763>";
		dbg_object(this).m_timeCode=dbg_object(this).m_timeCode+1;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<767>";
		if(dbg_object(this).m_enabled>1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<769>";
			this.p_OnDisabled();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<771>";
			dbg_object(this).m_enabled=dbg_object(this).m_enabled-1;
		}
	}
	pop_err();
}
c_iEngine.prototype.p_UpdateFixed=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<879>";
	if(dbg_object(this).m_paused){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<881>";
		this.p_UpdateSystemGui();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<882>";
		this.p_OnPaused();
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<886>";
		this.p_UpdateLogic();
	}
	pop_err();
}
c_iEngine.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<655>";
	c_iGuiObject.m_topObject=null;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<657>";
	if((bb_app2_iNextScene)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<659>";
		if(dbg_object(this).m_started){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<660>";
			bb_input_ResetInput();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<661>";
			dbg_object(this).m_started=false;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<662>";
			this.p_OnStop();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<665>";
		bb_app2_iNextScene.p_Set2();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<667>";
		bb_app2_iNextScene=null;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<669>";
		dbg_object(bb_globals_iEnginePointer).m_alphaFade=1.0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<670>";
		dbg_object(bb_globals_iEnginePointer).m_colorFade=1.0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<671>";
		dbg_object(bb_globals_iEnginePointer).m_countDown=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<672>";
		dbg_object(bb_globals_iEnginePointer).m_paused=false;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<673>";
		dbg_object(bb_globals_iEnginePointer).m_scoreCount=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<674>";
		dbg_object(bb_globals_iEnginePointer).m_timeCode=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<676>";
		if((c_iSystemGui.m_playfield)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<676>";
			c_iSystemGui.m_GuiPage(0);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<678>";
		if(dbg_object(bb_globals_iEnginePointer).m_cold==true){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<679>";
			bb_globals_iEnginePointer.p_OnColdStart();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<680>";
			dbg_object(bb_globals_iEnginePointer).m_cold=false;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<683>";
		if(dbg_object(bb_globals_iEnginePointer).m_started==false){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<684>";
			bb_globals_iEnginePointer.p_OnStart();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<685>";
			dbg_object(bb_globals_iEnginePointer).m_started=true;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<688>";
		if(dbg_object(bb_globals_iEnginePointer).m_paused){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<689>";
			bb_globals_iEnginePointer.p_UpdateSystemGui();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<690>";
			bb_globals_iEnginePointer.p_OnPaused();
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<692>";
			bb_globals_iEnginePointer.p_UpdateLogic();
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<697>";
		if(dbg_object(bb_globals_iEnginePointer).m_cold==true){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<698>";
			bb_globals_iEnginePointer.p_OnColdStart();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<699>";
			dbg_object(bb_globals_iEnginePointer).m_cold=false;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<702>";
		if(dbg_object(bb_globals_iEnginePointer).m_started==false){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<703>";
			bb_globals_iEnginePointer.p_OnStart();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<704>";
			dbg_object(bb_globals_iEnginePointer).m_started=true;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<707>";
		bb_globals_iEnginePointer.p_UpdateFixed();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/engine.monkey<711>";
	c_iTask.m_Update(bb_globals_iTaskList);
	pop_err();
}
function c_MenuScene(){
	c_iEngine.call(this);
}
c_MenuScene.prototype=extend_class(c_iEngine);
c_MenuScene.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<42>";
	c_iEngine.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<42>";
	pop_err();
	return this;
}
c_MenuScene.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<44>";
	print("Creating Menu");
	pop_err();
	return 0;
}
c_MenuScene.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<48>";
	bb_graphics_DrawText("Press Enter to Begin",200.0,200.0,0.0,0.0);
	pop_err();
	return 0;
}
c_MenuScene.prototype.p_OnStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<52>";
	print("Starting Menu");
	pop_err();
	return 0;
}
c_MenuScene.prototype.p_OnStop=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<56>";
	print("Stopping Menu");
	pop_err();
	return 0;
}
c_MenuScene.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<60>";
	if((bb_input_KeyHit(13))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<61>";
		print("Switch");
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<62>";
		bb_app2_iStart2((bb_main_gameplay),60);
	}
	pop_err();
	return 0;
}
var bb_main_menu=null;
function c_GameplayScene(){
	c_iEngine.call(this);
	this.m_playfield=null;
	this.m_layer=null;
	this.m_sprite1=null;
	this.m_p1=null;
	this.m_music=null;
	this.m_room=null;
}
c_GameplayScene.prototype=extend_class(c_iEngine);
c_GameplayScene.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<66>";
	c_iEngine.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<66>";
	pop_err();
	return this;
}
c_GameplayScene.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<78>";
	print("Creating Gameplay");
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<79>";
	dbg_object(this).m_playfield=c_iPlayfield.m_new.call(new c_iPlayfield);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<80>";
	dbg_object(this).m_playfield.p_AttachLast();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<81>";
	dbg_object(this).m_playfield.p_AutoCls2(0,0,0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<82>";
	dbg_object(dbg_object(this).m_playfield).m_width=600;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<83>";
	dbg_object(dbg_object(this).m_playfield).m_height=445;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<84>";
	dbg_object(this).m_playfield.p_Position(16.0,16.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<85>";
	dbg_object(this).m_playfield.p_ZoomPointX2(200.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<86>";
	dbg_object(this).m_playfield.p_ZoomPointY2(128.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<100>";
	dbg_object(this).m_layer=c_iLayer.m_new.call(new c_iLayer);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<104>";
	var t_img=bb_gfx_iLoadSprite2("char_walk_down.png",69,102,4);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<105>";
	dbg_object(this).m_layer.p_AttachLast3(dbg_object(this).m_playfield);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<109>";
	dbg_object(this).m_sprite1=c_iLayerSprite.m_new.call(new c_iLayerSprite);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<110>";
	dbg_object(this).m_sprite1.p_AttachLast4(dbg_object(this).m_layer);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<111>";
	dbg_object(this).m_sprite1.p_ImagePointer2(t_img);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<112>";
	dbg_object(this).m_sprite1.p_Position(300.0,275.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<120>";
	dbg_object(this).m_p1=c_Player.m_new.call(new c_Player,t_img,100,100);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<122>";
	this.m_music=bb_audio_LoadSound("tetris.mp3");
	pop_err();
	return 0;
}
c_GameplayScene.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<129>";
	this.m_room.p_Draw();
	pop_err();
	return 0;
}
c_GameplayScene.prototype.p_OnStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<142>";
	print("Starting Gameplay");
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<143>";
	this.m_room=c_Level.m_new.call(new c_Level,150,100,"Drunk");
	pop_err();
	return 0;
}
c_GameplayScene.prototype.p_OnStop=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<150>";
	print("Stopping Gameplay");
	pop_err();
	return 0;
}
c_GameplayScene.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<182>";
	if((bb_input_KeyDown(37))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<183>";
		dbg_object(this).m_playfield.p_CameraX2(dbg_object(this).m_playfield.p_CameraX()-4.0);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<186>";
	if((bb_input_KeyDown(39))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<187>";
		dbg_object(this).m_playfield.p_CameraX2(dbg_object(this).m_playfield.p_CameraX()+4.0);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<190>";
	if((bb_input_KeyDown(38))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<191>";
		dbg_object(this).m_playfield.p_CameraY2(dbg_object(this).m_playfield.p_CameraY()-4.0);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<194>";
	if((bb_input_KeyDown(40))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<195>";
		dbg_object(this).m_playfield.p_CameraY2(dbg_object(this).m_playfield.p_CameraY()+4.0);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<207>";
	this.m_sprite1.p_AnimationLoop(1.0,"60");
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<208>";
	dbg_object(this).m_sprite1.p_Show("LOOPING ANIMATION:");
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<209>";
	dbg_object(this).m_sprite1.p_Show("FramePointer="+String(dbg_object(this).m_sprite1.p_Frame2()));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/main.monkey<211>";
	this.m_p1.p_Update();
	pop_err();
	return 0;
}
var bb_main_gameplay=null;
var bb_app__updateRate=0;
function bb_app_UpdateRate(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<229>";
	pop_err();
	return bb_app__updateRate;
}
function bb_app_SetUpdateRate(t_hertz){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<224>";
	bb_app__updateRate=t_hertz;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/app.monkey<225>";
	bb_app__game.SetUpdateRate(t_hertz);
	pop_err();
}
function bb_app2_iStart(t_hertz){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<208>";
	if(t_hertz!=bb_app_UpdateRate()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<208>";
		bb_app_SetUpdateRate(t_hertz);
	}
	pop_err();
}
function bb_app2_iStart2(t_scene,t_hertz){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<216>";
	bb_app2_iNextScene=t_scene;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<218>";
	if(t_hertz!=bb_app_UpdateRate()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/app.monkey<218>";
		bb_app_SetUpdateRate(t_hertz);
	}
	pop_err();
}
var bb_globals_iEnginePointer=null;
function bb_functions_iTimeCode(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<412>";
	pop_err();
	return dbg_object(bb_globals_iEnginePointer).m_timeCode;
}
function c_iPlayfield(){
	c_iObject.call(this);
	this.m_visible=true;
	this.m_autoCls=true;
	this.m_x=.0;
	this.m_y=.0;
	this.m_width=0;
	this.m_scaleX=1.0;
	this.m_height=0;
	this.m_scaleY=1.0;
	this.m_alphaFade=1.0;
	this.m_colorFade=1.0;
	this.m_clsRed=.0;
	this.m_clsGreen=.0;
	this.m_clsBlue=.0;
	this.m_zoomPointX=.0;
	this.m_zoomPointY=.0;
	this.m_zoomX=1.0;
	this.m_zoomY=1.0;
	this.m_rotation=.0;
	this.m_cameraX=.0;
	this.m_cameraY=.0;
	this.m_layerList=c_iList2.m_new.call(new c_iList2);
	this.m_currentLayer=null;
	this.m_z=.0;
	this.m_guiEnabled=true;
	this.m_guiList=new_object_array(1);
	this.m_guiPage=0;
	this.m_guiLastObject=null;
	this.m_nextGuiPage=-1;
}
c_iPlayfield.prototype=extend_class(c_iObject);
c_iPlayfield.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1683>";
	var t_=(dbg_object(this).m_width)*dbg_object(this).m_scaleX;
	pop_err();
	return t_;
}
c_iPlayfield.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<497>";
	var t_=(dbg_object(this).m_height)*dbg_object(this).m_scaleY;
	pop_err();
	return t_;
}
c_iPlayfield.prototype.p_ClearBorders=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<302>";
	if(dbg_object(this).m_x==0.0 && dbg_object(this).m_y==0.0 && this.p_Width()==(bb_app_DeviceWidth()) && this.p_Height()==(bb_app_DeviceHeight())){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<304>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<306>";
	if(dbg_object(this).m_y>0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<307>";
		bb_gfx_iSetScissor(dbg_object(this).m_x,0.0,this.p_Width(),dbg_object(this).m_y);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<308>";
		bb_graphics_Cls((t_red),(t_green),(t_blue));
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<311>";
	if(dbg_object(this).m_y<(bb_app_DeviceHeight())-this.p_Height()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<312>";
		bb_gfx_iSetScissor(dbg_object(this).m_x,dbg_object(this).m_y+((this.p_Height())|0),this.p_Width(),(bb_app_DeviceHeight()-((dbg_object(this).m_y-this.p_Height())|0)));
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<313>";
		bb_graphics_Cls((t_red),(t_green),(t_blue));
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<316>";
	if(dbg_object(this).m_x>0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<317>";
		bb_gfx_iSetScissor(0.0,0.0,dbg_object(this).m_x,(bb_app_DeviceHeight()));
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<318>";
		bb_graphics_Cls((t_red),(t_green),(t_blue));
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<321>";
	if(dbg_object(this).m_x<(bb_app_DeviceWidth())-this.p_Width()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<322>";
		bb_gfx_iSetScissor(dbg_object(this).m_x+((this.p_Width())|0),0.0,(bb_app_DeviceWidth()-((dbg_object(this).m_x-this.p_Width())|0)),(bb_app_DeviceHeight()));
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<323>";
		bb_graphics_Cls((t_red),(t_green),(t_blue));
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<326>";
	c_iStack2D.m_Pop();
	pop_err();
}
c_iPlayfield.prototype.p_SetAlpha=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1186>";
	bb_graphics_SetAlpha(t_alpha*dbg_object(bb_globals_iEnginePointer).m_alphaFade*dbg_object(this).m_alphaFade);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1188>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_SetColor=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1198>";
	bb_graphics_SetColor(t_red*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(this).m_colorFade,t_green*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(this).m_colorFade,t_blue*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(this).m_colorFade);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1200>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_PositionX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<740>";
	pop_err();
	return dbg_object(this).m_x;
}
c_iPlayfield.prototype.p_PositionX2=function(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<747>";
	dbg_object(this).m_x=t_x;
	pop_err();
}
c_iPlayfield.prototype.p_PositionY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<754>";
	pop_err();
	return dbg_object(this).m_y;
}
c_iPlayfield.prototype.p_PositionY2=function(t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<761>";
	dbg_object(this).m_y=t_y;
	pop_err();
}
c_iPlayfield.prototype.p_ZoomPointX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1742>";
	pop_err();
	return dbg_object(this).m_zoomPointX;
}
c_iPlayfield.prototype.p_ZoomPointX2=function(t_zoomPointX){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1775>";
	dbg_object(this).m_zoomPointX=t_zoomPointX;
	pop_err();
}
c_iPlayfield.prototype.p_ZoomPointY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1746>";
	pop_err();
	return dbg_object(this).m_zoomPointY;
}
c_iPlayfield.prototype.p_ZoomPointY2=function(t_zoomPointY){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1782>";
	dbg_object(this).m_zoomPointY=t_zoomPointY;
	pop_err();
}
c_iPlayfield.prototype.p_ZoomX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1750>";
	pop_err();
	return dbg_object(this).m_zoomX;
}
c_iPlayfield.prototype.p_ZoomX2=function(t_zoomX){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1761>";
	dbg_object(this).m_zoomX=t_zoomX;
	pop_err();
}
c_iPlayfield.prototype.p_ZoomY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1754>";
	pop_err();
	return dbg_object(this).m_zoomY;
}
c_iPlayfield.prototype.p_ZoomY2=function(t_zoomY){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1768>";
	dbg_object(this).m_zoomY=t_zoomY;
	pop_err();
}
c_iPlayfield.prototype.p_Rotation=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1110>";
	pop_err();
	return dbg_object(this).m_rotation;
}
c_iPlayfield.prototype.p_Rotation2=function(t_rotation){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1117>";
	dbg_object(this).m_rotation=t_rotation;
	pop_err();
}
c_iPlayfield.prototype.p_CameraX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<252>";
	pop_err();
	return dbg_object(this).m_cameraX;
}
c_iPlayfield.prototype.p_CameraX2=function(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<259>";
	this.m_cameraX=t_x;
	pop_err();
}
c_iPlayfield.prototype.p_CameraY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<263>";
	pop_err();
	return dbg_object(this).m_cameraY;
}
c_iPlayfield.prototype.p_CameraY2=function(t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<270>";
	dbg_object(this).m_cameraY=t_y;
	pop_err();
}
c_iPlayfield.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<704>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_VWidth=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1654>";
	var t_=(dbg_object(this).m_width);
	pop_err();
	return t_;
}
c_iPlayfield.prototype.p_AlphaFade=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<196>";
	pop_err();
	return dbg_object(this).m_alphaFade;
}
c_iPlayfield.prototype.p_AlphaFade2=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<200>";
	dbg_object(this).m_alphaFade=t_alpha;
	pop_err();
}
c_iPlayfield.prototype.p_ColorFade=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<370>";
	pop_err();
	return dbg_object(this).m_colorFade;
}
c_iPlayfield.prototype.p_ColorFade2=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<374>";
	dbg_object(this).m_colorFade=t_value;
	pop_err();
}
c_iPlayfield.prototype.p_Position=function(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<720>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<721>";
	dbg_object(this).m_y=t_y;
	pop_err();
}
c_iPlayfield.prototype.p_Position2=function(t_x,t_y,t_z){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<730>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<731>";
	dbg_object(this).m_y=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<732>";
	dbg_object(this).m_z=t_z;
	pop_err();
}
c_iPlayfield.prototype.p_Zoom=function(t_zoom){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1718>";
	dbg_object(this).m_zoomX=dbg_object(this).m_zoomX+t_zoom;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1719>";
	dbg_object(this).m_zoomY=dbg_object(this).m_zoomY+t_zoom;
	pop_err();
}
c_iPlayfield.prototype.p_Zoom2=function(t_zoomX,t_zoomY){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1723>";
	this.p_ZoomX2(t_zoomX);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1724>";
	this.p_ZoomY2(t_zoomY);
	pop_err();
}
c_iPlayfield.prototype.p_VHeight=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1575>";
	var t_=(dbg_object(this).m_height);
	pop_err();
	return t_;
}
c_iPlayfield.prototype.p_ZoomPoint=function(t_zoomPointX,t_zoomPointY){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1736>";
	this.p_ZoomPointX2(t_zoomPointX);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1737>";
	this.p_ZoomPointY2(t_zoomPointY);
	pop_err();
}
c_iPlayfield.prototype.p_VHeight2=function(t_height){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1584>";
	this.p_Reset2D();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1586>";
	var t_h=(bb_app_DeviceHeight());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1588>";
	var t_s=t_h/(t_height);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1590>";
	dbg_object(this).m_width=(((bb_app_DeviceWidth())/t_s)|0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1591>";
	dbg_object(this).m_height=t_height;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1593>";
	dbg_object(this).m_scaleX=t_s;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1594>";
	dbg_object(this).m_scaleY=t_s;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1596>";
	this.p_ZoomPoint2();
	pop_err();
}
c_iPlayfield.prototype.p_ZoomPoint2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1729>";
	this.p_ZoomPointX2(this.p_VWidth()*.5);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1730>";
	this.p_ZoomPointY2(this.p_VHeight()*.5);
	pop_err();
}
c_iPlayfield.prototype.p_Reset2D=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1005>";
	dbg_object(this).m_width=bb_app_DeviceWidth();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1006>";
	dbg_object(this).m_height=bb_app_DeviceHeight();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1008>";
	this.p_AlphaFade2(1.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1009>";
	this.p_ColorFade2(1.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1010>";
	this.p_Position2(0.0,0.0,0.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1011>";
	this.p_Rotation2(0.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1012>";
	this.p_Zoom2(1.0,1.0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1014>";
	this.p_ZoomPoint2();
	pop_err();
}
c_iPlayfield.prototype.p_VWidth2=function(t_width){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1663>";
	this.p_Reset2D();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1665>";
	var t_w=(bb_app_DeviceWidth());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1667>";
	var t_s=t_w/(t_width);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1669>";
	dbg_object(this).m_width=t_width;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1670>";
	dbg_object(this).m_height=(((bb_app_DeviceHeight())/t_s)|0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1672>";
	dbg_object(this).m_scaleX=t_s;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1673>";
	dbg_object(this).m_scaleY=t_s;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1675>";
	this.p_ZoomPoint2();
	pop_err();
}
c_iPlayfield.prototype.p_OnTopRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<708>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_RenderGui=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<973>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<975>";
	if(dbg_object(bb_globals_iEnginePointer).m_borders){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<975>";
		bb_gfx_iSetScissor(this.p_PositionX(),this.p_PositionY(),(dbg_object(this).m_width)*dbg_object(this).m_scaleY,(dbg_object(this).m_height)*dbg_object(this).m_scaleY);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<977>";
	bb_graphics_Scale(dbg_object(this).m_scaleX,dbg_object(this).m_scaleY);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<979>";
	bb_graphics_Translate(this.p_PositionX()/dbg_object(this).m_scaleX,this.p_PositionY()/dbg_object(this).m_scaleY);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<981>";
	var t_g=dbg_array(dbg_object(this).m_guiList,dbg_object(this).m_guiPage)[dbg_index].p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<982>";
	while((t_g)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<984>";
		if(dbg_object(t_g).m_enabled==1 && dbg_object(t_g).m_visible){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<985>";
			if(dbg_object(t_g).m_ghost){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<985>";
				t_g.p_RenderGhost();
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<985>";
				t_g.p_Render();
			}
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<988>";
		t_g=dbg_array(dbg_object(this).m_guiList,dbg_object(this).m_guiPage)[dbg_index].p_Ascend();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<992>";
	if(((dbg_object(this).m_guiLastObject)!=null) && dbg_object(dbg_object(this).m_guiLastObject).m_enabled==1 && dbg_object(dbg_object(this).m_guiLastObject).m_visible){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<993>";
		if(dbg_object(dbg_object(this).m_guiLastObject).m_ghost){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<993>";
			dbg_object(this).m_guiLastObject.p_RenderGhost();
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<993>";
			dbg_object(this).m_guiLastObject.p_Render();
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<996>";
	c_iStack2D.m_Pop();
	pop_err();
}
c_iPlayfield.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<892>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<894>";
	if(dbg_object(bb_globals_iEnginePointer).m_borders){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<894>";
		bb_gfx_iSetScissor(this.p_PositionX(),this.p_PositionY(),(dbg_object(this).m_width)*dbg_object(this).m_scaleX,(dbg_object(this).m_height)*dbg_object(this).m_scaleY);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<896>";
	if(dbg_object(this).m_autoCls){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<896>";
		bb_graphics_Cls(dbg_object(this).m_clsRed*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(this).m_colorFade,dbg_object(this).m_clsGreen*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(this).m_colorFade,dbg_object(this).m_clsBlue*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(this).m_colorFade);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<898>";
	bb_graphics_Scale(dbg_object(this).m_scaleX,dbg_object(this).m_scaleY);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<900>";
	bb_graphics_Translate(this.p_PositionX()/dbg_object(this).m_scaleX+this.p_ZoomPointX(),this.p_PositionY()/dbg_object(this).m_scaleY+this.p_ZoomPointY());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<901>";
	bb_graphics_Scale(this.p_ZoomX(),this.p_ZoomY());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<902>";
	bb_graphics_Translate(-(this.p_PositionX()/dbg_object(this).m_scaleX+this.p_ZoomPointX()),-(this.p_PositionY()/dbg_object(this).m_scaleY+this.p_ZoomPointY()));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<904>";
	bb_graphics_Translate(this.p_PositionX()/dbg_object(this).m_scaleX+this.p_ZoomPointX(),this.p_PositionY()/dbg_object(this).m_scaleY+this.p_ZoomPointY());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<905>";
	bb_graphics_Rotate(this.p_Rotation()*(bb_globals_iRotation));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<906>";
	bb_graphics_Translate(-(this.p_PositionX()/dbg_object(this).m_scaleX+this.p_ZoomPointX()),-(this.p_PositionY()/dbg_object(this).m_scaleY+this.p_ZoomPointY()));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<908>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<910>";
	bb_graphics_Translate(this.p_PositionX()/dbg_object(this).m_scaleX-this.p_CameraX(),this.p_PositionY()/dbg_object(this).m_scaleY-this.p_CameraY());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<912>";
	if(dbg_object(bb_globals_iEnginePointer).m_renderToPlayfield==true){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<914>";
		c_iStack2D.m_Push();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<916>";
		bb_globals_iEnginePointer.p_SetAlpha(1.0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<917>";
		bb_globals_iEnginePointer.p_SetColor(255.0,255.0,255.0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<919>";
		bb_globals_iEnginePointer.p_OnRender();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<921>";
		c_iStack2D.m_Pop();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<925>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<926>";
	this.p_OnRender();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<927>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<929>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<931>";
	dbg_object(this).m_currentLayer=dbg_object(this).m_layerList.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<933>";
	while((dbg_object(this).m_currentLayer)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<935>";
		if(((dbg_object(dbg_object(this).m_currentLayer).m_enabled)!=0) && dbg_object(dbg_object(this).m_currentLayer).m_visible){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<935>";
			dbg_object(this).m_currentLayer.p_Render();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<937>";
		dbg_object(this).m_currentLayer=dbg_object(this).m_layerList.p_Ascend();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<941>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<943>";
	bb_graphics_Translate(this.p_PositionX()/dbg_object(this).m_scaleX-this.p_CameraX(),this.p_PositionY()/dbg_object(this).m_scaleY-this.p_CameraY());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<945>";
	if(dbg_object(bb_globals_iEnginePointer).m_renderToPlayfield==true){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<947>";
		c_iStack2D.m_Push();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<949>";
		bb_globals_iEnginePointer.p_SetAlpha(1.0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<950>";
		bb_globals_iEnginePointer.p_SetColor(255.0,255.0,255.0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<952>";
		bb_globals_iEnginePointer.p_OnTopRender();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<954>";
		c_iStack2D.m_Pop();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<958>";
	this.p_OnTopRender();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<960>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<962>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<964>";
	if(dbg_object(this).m_guiEnabled){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<964>";
		this.p_RenderGui();
	}
	pop_err();
}
c_iPlayfield.prototype.p_GuiPage=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<468>";
	if(dbg_object(this).m_nextGuiPage!=-1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<468>";
		pop_err();
		return this.m_nextGuiPage;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<468>";
		pop_err();
		return dbg_object(this).m_guiPage;
	}
}
c_iPlayfield.prototype.p_InitGuiPages=function(t_pages){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<523>";
	if(dbg_object(this).m_guiList.length<=t_pages){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<525>";
		dbg_object(this).m_guiList=resize_object_array(dbg_object(this).m_guiList,t_pages+1);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<527>";
		for(var t_p=0;t_p<dbg_object(this).m_guiList.length;t_p=t_p+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<528>";
			if(!((dbg_array(dbg_object(this).m_guiList,t_p)[dbg_index])!=null)){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<528>";
				dbg_array(dbg_object(this).m_guiList,t_p)[dbg_index]=c_iList4.m_new.call(new c_iList4);
			}
		}
	}
	pop_err();
}
c_iPlayfield.prototype.p_GuiPage2=function(t_page){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<475>";
	this.p_InitGuiPages(t_page);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<476>";
	dbg_object(this).m_nextGuiPage=t_page;
	pop_err();
}
c_iPlayfield.prototype.p_Set2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1178>";
	if((bb_globals_iEnginePointer)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1178>";
		dbg_object(bb_globals_iEnginePointer).m_playfieldPointer=this;
	}
	pop_err();
}
c_iPlayfield.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<700>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_SystemInit=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1426>";
	this.p_Set2();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1428>";
	dbg_object(this).m_height=bb_app_DeviceHeight();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1429>";
	dbg_object(this).m_width=bb_app_DeviceWidth();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1431>";
	dbg_array(dbg_object(this).m_guiList,0)[dbg_index]=c_iList4.m_new.call(new c_iList4);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1433>";
	this.p_ZoomPoint2();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1435>";
	this.p_OnCreate();
	pop_err();
}
c_iPlayfield.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<669>";
	c_iObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<670>";
	this.p_SystemInit();
	pop_err();
	return this;
}
c_iPlayfield.m_new2=function(t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<673>";
	c_iObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<675>";
	this.p_SystemInit();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<677>";
	dbg_object(this).m_width=t_width;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<678>";
	dbg_object(this).m_height=t_height;
	pop_err();
	return this;
}
c_iPlayfield.m_new3=function(t_x,t_y,t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<682>";
	c_iObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<684>";
	this.p_SystemInit();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<686>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<687>";
	dbg_object(this).m_y=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<689>";
	dbg_object(this).m_width=t_width;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<690>";
	dbg_object(this).m_height=t_height;
	pop_err();
	return this;
}
c_iPlayfield.prototype.p_UpdateGui=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1482>";
	if(dbg_object(this).m_nextGuiPage>=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1483>";
		dbg_object(this).m_guiPage=dbg_object(this).m_nextGuiPage;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1484>";
		dbg_object(this).m_nextGuiPage=-1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1487>";
	if((dbg_object(this).m_guiLastObject)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1489>";
		dbg_object(this).m_guiLastObject.p_UpdateWorldXY();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1491>";
		if(dbg_object(dbg_object(this).m_guiLastObject).m_enabled==1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1493>";
			if(dbg_object(dbg_object(this).m_guiLastObject).m_ghost){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1495>";
				dbg_object(this).m_guiLastObject.p_UpdateGhost();
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1499>";
				dbg_object(this).m_guiLastObject.p_UpdateInput();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1500>";
				dbg_object(this).m_guiLastObject.p_Update();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1502>";
				if((dbg_object(this).m_guiLastObject)!=null){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1502>";
					dbg_object(this).m_guiLastObject.p_OnUpdate();
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1510>";
		var t_g=null;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1512>";
		t_g=dbg_array(dbg_object(this).m_guiList,dbg_object(this).m_guiPage)[dbg_index].p_Last();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1514>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1516>";
			t_g.p_UpdateWorldXY();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1518>";
			if(((dbg_object(t_g).m_enabled)!=0) && !dbg_object(t_g).m_ghost){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1518>";
				t_g.p_UpdateInput();
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1520>";
			t_g=dbg_array(dbg_object(this).m_guiList,dbg_object(this).m_guiPage)[dbg_index].p_Descend();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1524>";
		t_g=dbg_array(this.m_guiList,dbg_object(this).m_guiPage)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1525>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1527>";
			if(dbg_object(t_g).m_enabled==1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1529>";
				if(dbg_object(t_g).m_ghost){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1531>";
					t_g.p_UpdateGhost();
				}else{
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1535>";
					t_g.p_Update();
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1536>";
					t_g.p_OnUpdate();
				}
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1542>";
			t_g=dbg_array(dbg_object(this).m_guiList,dbg_object(this).m_guiPage)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iPlayfield.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<712>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1472>";
	this.p_UpdateGui();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<1473>";
	this.p_OnUpdate();
	pop_err();
}
c_iPlayfield.prototype.p_AttachLast=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<221>";
	dbg_object(bb_globals_iEnginePointer).m_playfieldList.p_AddLast(this);
	pop_err();
}
c_iPlayfield.prototype.p_AttachLast2=function(t_engine){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<228>";
	dbg_object(t_engine).m_playfieldList.p_AddLast(this);
	pop_err();
}
c_iPlayfield.prototype.p_AutoCls2=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<236>";
	dbg_object(this).m_clsRed=(t_red);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<237>";
	dbg_object(this).m_clsGreen=(t_green);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<238>";
	dbg_object(this).m_clsBlue=(t_blue);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<240>";
	dbg_object(this).m_autoCls=true;
	pop_err();
}
c_iPlayfield.prototype.p_AutoCls3=function(t_state){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/playfield.monkey<248>";
	dbg_object(this).m_autoCls=t_state;
	pop_err();
}
function c_iList(){
	Object.call(this);
	this.m_index=0;
	this.m_length=0;
	this.m_data=[];
}
c_iList.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<97>";
	dbg_object(this).m_index=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
	if((dbg_object(this).m_length)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<67>";
	dbg_object(this).m_index=dbg_object(this).m_index+1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
	if(dbg_object(this).m_index>=0 && dbg_object(this).m_index<dbg_object(this).m_length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,dbg_object(this).m_index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList.prototype.p_Length=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<147>";
	pop_err();
	return dbg_object(this).m_length;
}
c_iList.prototype.p_Get=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<106>";
	pop_err();
	return dbg_array(dbg_object(this).m_data,t_index)[dbg_index];
}
c_iList.prototype.p_AddLast=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<55>";
	if(dbg_object(this).m_length==dbg_object(this).m_data.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<56>";
		dbg_object(this).m_data=resize_object_array(dbg_object(this).m_data,dbg_object(this).m_length*2+10);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<58>";
	dbg_array(dbg_object(this).m_data,dbg_object(this).m_length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<59>";
	dbg_object(this).m_length=dbg_object(this).m_length+1;
	pop_err();
}
function bb_graphics_GetScissor(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<297>";
	var t_=[dbg_object(bb_graphics_context).m_scissor_x,dbg_object(bb_graphics_context).m_scissor_y,dbg_object(bb_graphics_context).m_scissor_width,dbg_object(bb_graphics_context).m_scissor_height];
	pop_err();
	return t_;
}
function bb_graphics_GetScissor2(t_scissor){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<301>";
	dbg_array(t_scissor,0)[dbg_index]=dbg_object(bb_graphics_context).m_scissor_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<302>";
	dbg_array(t_scissor,1)[dbg_index]=dbg_object(bb_graphics_context).m_scissor_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<303>";
	dbg_array(t_scissor,2)[dbg_index]=dbg_object(bb_graphics_context).m_scissor_width;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<304>";
	dbg_array(t_scissor,3)[dbg_index]=dbg_object(bb_graphics_context).m_scissor_height;
	pop_err();
	return 0;
}
function bb_graphics_DebugRenderDevice(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<53>";
	if(!((bb_graphics_renderDevice)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<53>";
		error("Rendering operations can only be performed inside OnRender");
	}
	pop_err();
	return 0;
}
function bb_graphics_Cls(t_r,t_g,t_b){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<378>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<380>";
	bb_graphics_renderDevice.Cls(t_r,t_g,t_b);
	pop_err();
	return 0;
}
function bb_gfx_iCls(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<19>";
	var t_s=bb_graphics_GetScissor();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<21>";
	bb_graphics_SetScissor(0.0,0.0,(bb_app_DeviceWidth()),(bb_app_DeviceHeight()));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<22>";
	bb_graphics_Cls(t_red,t_green,t_blue);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<24>";
	bb_graphics_SetScissor(dbg_array(t_s,0)[dbg_index],dbg_array(t_s,1)[dbg_index],dbg_array(t_s,2)[dbg_index],dbg_array(t_s,3)[dbg_index]);
	pop_err();
}
function c_iStack2D(){
	Object.call(this);
	this.m_alpha=.0;
	this.m_blend=0;
	this.m_color=[];
	this.m_matrix=[];
	this.m_scissor=[];
}
c_iStack2D.m_length=0;
c_iStack2D.m_data=[];
c_iStack2D.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<10>";
	pop_err();
	return this;
}
c_iStack2D.m_Push=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<48>";
	if(c_iStack2D.m_length==c_iStack2D.m_data.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<50>";
		c_iStack2D.m_data=resize_object_array(c_iStack2D.m_data,c_iStack2D.m_length*2+10);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<52>";
		for(var t_i=0;t_i<c_iStack2D.m_data.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<53>";
			if(!((dbg_array(c_iStack2D.m_data,t_i)[dbg_index])!=null)){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<53>";
				dbg_array(c_iStack2D.m_data,t_i)[dbg_index]=c_iStack2D.m_new.call(new c_iStack2D);
			}
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<58>";
	var t_o=dbg_array(c_iStack2D.m_data,c_iStack2D.m_length)[dbg_index];
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<60>";
	dbg_object(t_o).m_alpha=bb_graphics_GetAlpha();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<61>";
	dbg_object(t_o).m_blend=bb_graphics_GetBlend();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<62>";
	dbg_object(t_o).m_color=bb_graphics_GetColor();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<63>";
	dbg_object(t_o).m_matrix=bb_graphics_GetMatrix();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<64>";
	dbg_object(t_o).m_scissor=bb_graphics_GetScissor();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<66>";
	c_iStack2D.m_length=c_iStack2D.m_length+1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<68>";
	pop_err();
	return t_o;
}
c_iStack2D.m_Pop=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<34>";
	c_iStack2D.m_length=c_iStack2D.m_length-1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<36>";
	var t_o=dbg_array(c_iStack2D.m_data,c_iStack2D.m_length)[dbg_index];
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<38>";
	bb_graphics_SetAlpha(dbg_object(t_o).m_alpha);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<39>";
	bb_graphics_SetBlend(dbg_object(t_o).m_blend);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<40>";
	bb_graphics_SetMatrix(dbg_array(dbg_object(t_o).m_matrix,0)[dbg_index],dbg_array(dbg_object(t_o).m_matrix,1)[dbg_index],dbg_array(dbg_object(t_o).m_matrix,2)[dbg_index],dbg_array(dbg_object(t_o).m_matrix,3)[dbg_index],dbg_array(dbg_object(t_o).m_matrix,4)[dbg_index],dbg_array(dbg_object(t_o).m_matrix,5)[dbg_index]);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<41>";
	bb_graphics_SetColor(dbg_array(dbg_object(t_o).m_color,0)[dbg_index],dbg_array(dbg_object(t_o).m_color,1)[dbg_index],dbg_array(dbg_object(t_o).m_color,2)[dbg_index]);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/stack2d.monkey<42>";
	bb_graphics_SetScissor(dbg_array(dbg_object(t_o).m_scissor,0)[dbg_index],dbg_array(dbg_object(t_o).m_scissor,1)[dbg_index],dbg_array(dbg_object(t_o).m_scissor,2)[dbg_index],dbg_array(dbg_object(t_o).m_scissor,3)[dbg_index]);
	pop_err();
}
function bb_graphics_GetAlpha(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<276>";
	pop_err();
	return dbg_object(bb_graphics_context).m_alpha;
}
function bb_graphics_GetBlend(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<285>";
	pop_err();
	return dbg_object(bb_graphics_context).m_blend;
}
function bb_graphics_GetColor(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<261>";
	var t_=[dbg_object(bb_graphics_context).m_color_r,dbg_object(bb_graphics_context).m_color_g,dbg_object(bb_graphics_context).m_color_b];
	pop_err();
	return t_;
}
function bb_graphics_GetColor2(t_color){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<265>";
	dbg_array(t_color,0)[dbg_index]=dbg_object(bb_graphics_context).m_color_r;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<266>";
	dbg_array(t_color,1)[dbg_index]=dbg_object(bb_graphics_context).m_color_g;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<267>";
	dbg_array(t_color,2)[dbg_index]=dbg_object(bb_graphics_context).m_color_b;
	pop_err();
	return 0;
}
function bb_graphics_GetMatrix(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<323>";
	var t_=[dbg_object(bb_graphics_context).m_ix,dbg_object(bb_graphics_context).m_iy,dbg_object(bb_graphics_context).m_jx,dbg_object(bb_graphics_context).m_jy,dbg_object(bb_graphics_context).m_tx,dbg_object(bb_graphics_context).m_ty];
	pop_err();
	return t_;
}
function bb_graphics_GetMatrix2(t_matrix){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<327>";
	dbg_array(t_matrix,0)[dbg_index]=dbg_object(bb_graphics_context).m_ix;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<327>";
	dbg_array(t_matrix,1)[dbg_index]=dbg_object(bb_graphics_context).m_iy;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<328>";
	dbg_array(t_matrix,2)[dbg_index]=dbg_object(bb_graphics_context).m_jx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<328>";
	dbg_array(t_matrix,3)[dbg_index]=dbg_object(bb_graphics_context).m_jy;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<329>";
	dbg_array(t_matrix,4)[dbg_index]=dbg_object(bb_graphics_context).m_tx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<329>";
	dbg_array(t_matrix,5)[dbg_index]=dbg_object(bb_graphics_context).m_ty;
	pop_err();
	return 0;
}
function bb_gfx_iSetScissor(t_x,t_y,t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<498>";
	if(t_x<0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<500>";
		if(-t_x>=t_width){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<501>";
			t_x=0.0;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<502>";
			t_width=0.0;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<504>";
			t_width=t_width+t_x;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<505>";
			t_x=0.0;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<510>";
	if(t_x+t_width>(bb_app_DeviceWidth())){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<512>";
		if(t_x>=(bb_app_DeviceWidth())){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<513>";
			t_x=(bb_app_DeviceWidth());
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<514>";
			t_width=0.0;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<516>";
			t_width=(bb_app_DeviceWidth()-((t_x)|0));
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<521>";
	if(t_y<0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<523>";
		if(-t_y>=t_height){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<524>";
			t_y=0.0;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<525>";
			t_height=0.0;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<527>";
			t_height=t_height+t_y;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<528>";
			t_y=0.0;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<533>";
	if(t_y+t_height>(bb_app_DeviceHeight())){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<535>";
		if(t_y>=(bb_app_DeviceHeight())){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<536>";
			t_y=(bb_app_DeviceHeight());
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<537>";
			t_height=0.0;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<539>";
			t_height=(bb_app_DeviceHeight()-((t_y)|0));
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/gfx.monkey<544>";
	bb_graphics_SetScissor(t_x,t_y,t_width,t_height);
	pop_err();
}
function bb_graphics_Transform(t_ix,t_iy,t_jx,t_jy,t_tx,t_ty){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<355>";
	var t_ix2=t_ix*dbg_object(bb_graphics_context).m_ix+t_iy*dbg_object(bb_graphics_context).m_jx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<356>";
	var t_iy2=t_ix*dbg_object(bb_graphics_context).m_iy+t_iy*dbg_object(bb_graphics_context).m_jy;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<357>";
	var t_jx2=t_jx*dbg_object(bb_graphics_context).m_ix+t_jy*dbg_object(bb_graphics_context).m_jx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<358>";
	var t_jy2=t_jx*dbg_object(bb_graphics_context).m_iy+t_jy*dbg_object(bb_graphics_context).m_jy;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<359>";
	var t_tx2=t_tx*dbg_object(bb_graphics_context).m_ix+t_ty*dbg_object(bb_graphics_context).m_jx+dbg_object(bb_graphics_context).m_tx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<360>";
	var t_ty2=t_tx*dbg_object(bb_graphics_context).m_iy+t_ty*dbg_object(bb_graphics_context).m_jy+dbg_object(bb_graphics_context).m_ty;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<361>";
	bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
	pop_err();
	return 0;
}
function bb_graphics_Transform2(t_m){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<351>";
	bb_graphics_Transform(dbg_array(t_m,0)[dbg_index],dbg_array(t_m,1)[dbg_index],dbg_array(t_m,2)[dbg_index],dbg_array(t_m,3)[dbg_index],dbg_array(t_m,4)[dbg_index],dbg_array(t_m,5)[dbg_index]);
	pop_err();
	return 0;
}
function bb_graphics_Scale(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<369>";
	bb_graphics_Transform(t_x,0.0,0.0,t_y,0.0,0.0);
	pop_err();
	return 0;
}
function bb_graphics_Translate(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<365>";
	bb_graphics_Transform(1.0,0.0,0.0,1.0,t_x,t_y);
	pop_err();
	return 0;
}
var bb_globals_iRotation=0;
function bb_graphics_Rotate(t_angle){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<373>";
	bb_graphics_Transform(Math.cos((t_angle)*D2R),-Math.sin((t_angle)*D2R),Math.sin((t_angle)*D2R),Math.cos((t_angle)*D2R),0.0,0.0);
	pop_err();
	return 0;
}
function c_iLayer(){
	c_iObject.call(this);
	this.m_visible=true;
	this.m_stack2D=null;
	this.m_cameraSpeedX=1.0;
	this.m_cameraSpeedY=1.0;
	this.m_objectList=[];
	this.m_blockWidth=0;
	this.m_blockWidthExtra=0;
	this.m_blockHeight=0;
	this.m_blockHeightExtra=0;
	this.m_timeStamp=0;
	this.m_alphaFade=1.0;
	this.m_colorFade=1.0;
}
c_iLayer.prototype=extend_class(c_iObject);
c_iLayer.prototype.p_CameraSpeedX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<165>";
	pop_err();
	return dbg_object(this).m_cameraSpeedX;
}
c_iLayer.prototype.p_CameraSpeedX2=function(t_speedX){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<172>";
	dbg_object(this).m_cameraSpeedX=t_speedX;
	pop_err();
}
c_iLayer.prototype.p_CameraSpeedY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<179>";
	pop_err();
	return dbg_object(this).m_cameraSpeedY;
}
c_iLayer.prototype.p_CameraSpeedY2=function(t_speedY){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<186>";
	dbg_object(this).m_cameraSpeedY=t_speedY;
	pop_err();
}
c_iLayer.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<574>";
	pop_err();
	return 0;
}
c_iLayer.prototype.p_Grid=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<319>";
	if(this.m_objectList.length>1 || dbg_array(this.m_objectList,0)[dbg_index].length>1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<319>";
		pop_err();
		return true;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<319>";
		pop_err();
		return false;
	}
}
c_iLayer.prototype.p_GridWidth=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<354>";
	var t_=dbg_object(this).m_objectList.length;
	pop_err();
	return t_;
}
c_iLayer.prototype.p_GridHeight=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<326>";
	var t_=dbg_array(dbg_object(this).m_objectList,0)[dbg_index].length;
	pop_err();
	return t_;
}
c_iLayer.prototype.p_RenderLayerObjects=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<705>";
	if(this.p_Grid()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<707>";
		var t_startX=((dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_cameraX/(this.m_blockWidth)-(this.m_blockWidthExtra))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<708>";
		var t_startY=((dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_cameraY/(this.m_blockHeight)-(this.m_blockHeightExtra))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<710>";
		if(t_startX<0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<710>";
			t_startX=0;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<711>";
		if(t_startY<0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<711>";
			t_startY=0;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<713>";
		var t_stopX=(((t_startX)+dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_VWidth()/(this.m_blockWidth)+1.0+1.0+(this.m_blockWidthExtra*2))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<714>";
		var t_stopY=(((t_startY)+dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_VHeight()/(this.m_blockHeight)+1.0+1.0+(this.m_blockHeightExtra*2))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<716>";
		if(t_stopX>this.p_GridWidth()){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<716>";
			t_stopX=this.p_GridWidth();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<717>";
		if(t_stopY>this.p_GridHeight()){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<717>";
			t_stopY=this.p_GridHeight();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<719>";
		for(var t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<721>";
			for(var t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<723>";
				var t_o=dbg_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_y)[dbg_index].p_First();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<724>";
				while((t_o)!=null){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<726>";
					if(dbg_object(t_o).m_enabled==1 && dbg_object(t_o).m_visible){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<726>";
						t_o.p_Render();
					}
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<728>";
					t_o=dbg_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_y)[dbg_index].p_Ascend();
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<738>";
		var t_o2=dbg_array(dbg_array(dbg_object(this).m_objectList,0)[dbg_index],0)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<739>";
		while((t_o2)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<741>";
			if(dbg_object(t_o2).m_enabled==1 && dbg_object(t_o2).m_visible){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<741>";
				t_o2.p_Render();
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<743>";
			t_o2=dbg_array(dbg_array(dbg_object(this).m_objectList,0)[dbg_index],0)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iLayer.prototype.p_OnTopRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<578>";
	pop_err();
	return 0;
}
c_iLayer.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<668>";
	this.m_stack2D=c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<670>";
	if(this.p_CameraSpeedX()!=0.0 || this.p_CameraSpeedY()!=0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<671>";
		bb_graphics_Translate(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_PositionX()/dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_scaleX-dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_CameraX()*dbg_object(dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_currentLayer).m_cameraSpeedX,dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_PositionY()/dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_scaleY-dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_CameraY()*dbg_object(dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_currentLayer).m_cameraSpeedY);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<674>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<675>";
	this.p_OnRender();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<676>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<678>";
	this.p_RenderLayerObjects();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<680>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<681>";
	this.p_OnTopRender();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<682>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<684>";
	c_iStack2D.m_Pop();
	pop_err();
}
c_iLayer.prototype.p_UpdateWorldXY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<944>";
	if(this.p_Grid()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<946>";
		var t_startX=((dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_cameraX/(this.m_blockWidth)-(this.m_blockWidthExtra))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<947>";
		var t_startY=((dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_cameraY/(this.m_blockHeight)-(this.m_blockHeightExtra))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<949>";
		if(t_startX<0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<949>";
			t_startX=0;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<950>";
		if(t_startY<0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<950>";
			t_startY=0;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<952>";
		var t_stopX=(((t_startX)+dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_VWidth()/(this.m_blockWidth)+1.0+1.0+(this.m_blockWidthExtra*2))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<953>";
		var t_stopY=(((t_startY)+dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_VHeight()/(this.m_blockHeight)+1.0+1.0-(this.m_blockHeightExtra*2))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<955>";
		if(t_stopX>this.p_GridWidth()){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<955>";
			t_stopX=this.p_GridWidth();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<956>";
		if(t_stopY>this.p_GridHeight()){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<956>";
			t_stopY=this.p_GridHeight();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<958>";
		for(var t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<960>";
			for(var t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<962>";
				var t_o=dbg_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_y)[dbg_index].p_First();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<963>";
				while((t_o)!=null){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<965>";
					t_o.p_UpdateWorldXY();
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<967>";
					t_o=dbg_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_y)[dbg_index].p_Ascend();
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<977>";
		var t_o2=dbg_array(dbg_array(dbg_object(this).m_objectList,0)[dbg_index],0)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<978>";
		while((t_o2)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<980>";
			t_o2.p_UpdateWorldXY();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<982>";
			t_o2=dbg_array(dbg_array(dbg_object(this).m_objectList,0)[dbg_index],0)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iLayer.prototype.p_UpdateLayerObjects=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<871>";
	if(this.p_Grid()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<873>";
		var t_startX=((dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_cameraX/(this.m_blockWidth)-(this.m_blockWidthExtra))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<874>";
		var t_startY=((dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_cameraY/(this.m_blockHeight)-(this.m_blockHeightExtra))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<876>";
		if(t_startX<0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<876>";
			t_startX=0;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<877>";
		if(t_startY<0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<877>";
			t_startY=0;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<879>";
		var t_stopX=(((t_startX)+dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_VWidth()/(this.m_blockWidth)+1.0+1.0+(this.m_blockWidthExtra*2))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<880>";
		var t_stopY=(((t_startY)+dbg_object(bb_globals_iEnginePointer).m_currentPlayfield.p_VHeight()/(this.m_blockHeight)+1.0+1.0-(this.m_blockHeightExtra*2))|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<882>";
		if(t_stopX>this.p_GridWidth()){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<882>";
			t_stopX=this.p_GridWidth();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<883>";
		if(t_stopY>this.p_GridHeight()){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<883>";
			t_stopY=this.p_GridHeight();
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<885>";
		for(var t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<887>";
			for(var t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<889>";
				var t_o=dbg_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_y)[dbg_index].p_First();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<890>";
				while((t_o)!=null){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<892>";
					if(dbg_object(t_o).m_enabled==1){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<894>";
						t_o.p_Update();
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<895>";
						t_o.p_OnUpdate();
					}else{
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<899>";
						t_o.p_OnDisabled();
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<901>";
						if(dbg_object(t_o).m_enabled>1){
							err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<901>";
							dbg_object(t_o).m_enabled=dbg_object(t_o).m_enabled-1;
						}
					}
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<905>";
					t_o=dbg_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_y)[dbg_index].p_Ascend();
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<915>";
		var t_o2=dbg_array(dbg_array(dbg_object(this).m_objectList,0)[dbg_index],0)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<916>";
		while((t_o2)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<918>";
			if(dbg_object(t_o2).m_enabled==1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<920>";
				t_o2.p_Update();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<921>";
				t_o2.p_OnUpdate();
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<925>";
				t_o2.p_OnDisabled();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<927>";
				if(dbg_object(t_o2).m_enabled>1){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<927>";
					dbg_object(t_o2).m_enabled=dbg_object(t_o2).m_enabled-1;
				}
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<931>";
			t_o2=dbg_array(dbg_array(dbg_object(this).m_objectList,0)[dbg_index],0)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iLayer.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<582>";
	pop_err();
	return 0;
}
c_iLayer.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<858>";
	this.p_UpdateWorldXY();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<860>";
	this.p_UpdateLayerObjects();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<862>";
	this.p_OnUpdate();
	pop_err();
}
c_iLayer.prototype.p_Set2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<781>";
	if((bb_globals_iEnginePointer)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<781>";
		dbg_object(bb_globals_iEnginePointer).m_layerPointer=this;
	}
	pop_err();
}
c_iLayer.prototype.p_InitGrid=function(t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<434>";
	dbg_object(this).m_objectList=resize_array_array(dbg_object(this).m_objectList,t_width);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<436>";
	for(var t_x=0;t_x<t_width;t_x=t_x+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<438>";
		if(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index].length<t_height){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<438>";
			dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index]=resize_object_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_height);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<440>";
		for(var t_y=0;t_y<t_height;t_y=t_y+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<441>";
			if(!((dbg_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_y)[dbg_index])!=null)){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<441>";
				dbg_array(dbg_array(dbg_object(this).m_objectList,t_x)[dbg_index],t_y)[dbg_index]=c_iList3.m_new.call(new c_iList3);
			}
		}
	}
	pop_err();
}
c_iLayer.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<570>";
	pop_err();
	return 0;
}
c_iLayer.prototype.p_SystemInit=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<821>";
	this.p_Set2();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<823>";
	this.p_InitGrid(1,1);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<825>";
	this.p_OnCreate();
	pop_err();
}
c_iLayer.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<552>";
	c_iObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<553>";
	this.p_SystemInit();
	pop_err();
	return this;
}
c_iLayer.prototype.p_AttachLast=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<143>";
	dbg_object(dbg_object(bb_globals_iEnginePointer).m_playfieldPointer).m_layerList.p_AddLast2(this);
	pop_err();
}
c_iLayer.prototype.p_AttachLast3=function(t_playfield){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layer.monkey<150>";
	dbg_object(t_playfield).m_layerList.p_AddLast2(this);
	pop_err();
}
function c_iList2(){
	Object.call(this);
	this.m_index=0;
	this.m_length=0;
	this.m_data=[];
}
c_iList2.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList2.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<97>";
	dbg_object(this).m_index=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
	if((dbg_object(this).m_length)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList2.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<67>";
	dbg_object(this).m_index=dbg_object(this).m_index+1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
	if(dbg_object(this).m_index>=0 && dbg_object(this).m_index<dbg_object(this).m_length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,dbg_object(this).m_index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList2.prototype.p_AddLast2=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<55>";
	if(dbg_object(this).m_length==dbg_object(this).m_data.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<56>";
		dbg_object(this).m_data=resize_object_array(dbg_object(this).m_data,dbg_object(this).m_length*2+10);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<58>";
	dbg_array(dbg_object(this).m_data,dbg_object(this).m_length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<59>";
	dbg_object(this).m_length=dbg_object(this).m_length+1;
	pop_err();
}
function c_iGraph(){
	c_iObject.call(this);
	this.m_visible=true;
	this.m_ghost=false;
	this.m_height=0;
	this.m_scaleY=1.0;
	this.m_scaleX=1.0;
	this.m_rotation=.0;
	this.m_width=0;
	this.m_x=.0;
	this.m_y=.0;
	this.m_mcPosition=c_iVector2d.m_new.call(new c_iVector2d,0.0,0.0);
	this.m_mcVelocity=c_iVector2d.m_new.call(new c_iVector2d,0.0,0.0);
	this.m_z=.0;
	this.m_ghostBlend=0;
	this.m_ghostAlpha=1.0;
	this.m_ghostRed=255.0;
	this.m_ghostGreen=255.0;
	this.m_ghostBlue=255.0;
	this.m_blend=0;
	this.m_alpha=1.0;
	this.m_red=255.0;
	this.m_green=255.0;
	this.m_blue=255.0;
	this.implments={c_IMover:1};
}
c_iGraph.prototype=extend_class(c_iObject);
c_iGraph.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<439>";
	var t_=(dbg_object(this).m_height)*dbg_object(this).m_scaleY;
	pop_err();
	return t_;
}
c_iGraph.prototype.p_ScaleX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<576>";
	pop_err();
	return dbg_object(this).m_scaleX;
}
c_iGraph.prototype.p_ScaleX2=function(t_scaleX){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<583>";
	dbg_object(this).m_scaleX=t_scaleX;
	pop_err();
}
c_iGraph.prototype.p_Rotation=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<552>";
	pop_err();
	return dbg_object(this).m_rotation;
}
c_iGraph.prototype.p_Rotation2=function(t_angle){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<559>";
	dbg_object(this).m_rotation=t_angle;
	pop_err();
}
c_iGraph.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<620>";
	var t_=(dbg_object(this).m_width)*dbg_object(this).m_scaleX;
	pop_err();
	return t_;
}
c_iGraph.prototype.p_PositionX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<446>";
	pop_err();
	return dbg_object(this).m_x;
}
c_iGraph.prototype.p_PositionX2=function(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<453>";
	dbg_object(this).m_x=t_x;
	pop_err();
}
c_iGraph.prototype.p_PositionX3=function(t_x,t_graph){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<460>";
	dbg_object(this).m_x=dbg_object(t_graph).m_x+t_x;
	pop_err();
}
c_iGraph.prototype.p_PositionY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<467>";
	pop_err();
	return dbg_object(this).m_y;
}
c_iGraph.prototype.p_PositionY2=function(t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<474>";
	dbg_object(this).m_y=t_y;
	pop_err();
}
c_iGraph.prototype.p_PositionY3=function(t_y,t_graph){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<481>";
	dbg_object(this).m_y=dbg_object(t_graph).m_y+t_y;
	pop_err();
}
c_iGraph.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<361>";
	this.m_mcPosition=null;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<362>";
	this.m_mcVelocity=null;
	pop_err();
}
c_iGraph.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<10>";
	c_iObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<10>";
	pop_err();
	return this;
}
c_iGraph.prototype.p_PositionZ=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<488>";
	pop_err();
	return dbg_object(this).m_z;
}
c_iGraph.prototype.p_PositionZ2=function(t_z){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<495>";
	dbg_object(this).m_z=t_z;
	pop_err();
}
c_iGraph.prototype.p_PositionZ3=function(t_z,t_graph){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<502>";
	dbg_object(this).m_z=dbg_object(t_graph).m_z+t_z;
	pop_err();
}
c_iGraph.prototype.p_ScaleY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<590>";
	pop_err();
	return dbg_object(this).m_scaleY;
}
c_iGraph.prototype.p_ScaleY2=function(t_scaleY){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<597>";
	dbg_object(this).m_scaleY=t_scaleY;
	pop_err();
}
c_iGraph.prototype.p_Alpha=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<297>";
	pop_err();
	return dbg_object(this).m_alpha;
}
c_iGraph.prototype.p_Alpha2=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<306>";
	dbg_object(this).m_alpha=t_alpha;
	pop_err();
}
c_iGraph.prototype.p_Red=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<509>";
	pop_err();
	return dbg_object(this).m_red;
}
c_iGraph.prototype.p_Red2=function(t_red){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<518>";
	dbg_object(this).m_red=t_red;
	pop_err();
}
c_iGraph.prototype.p_Green=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<423>";
	pop_err();
	return dbg_object(this).m_green;
}
c_iGraph.prototype.p_Green2=function(t_green){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<432>";
	dbg_object(this).m_green=t_green;
	pop_err();
}
c_iGraph.prototype.p_Blue=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<321>";
	pop_err();
	return dbg_object(this).m_blue;
}
c_iGraph.prototype.p_Blue2=function(t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/graph.monkey<330>";
	dbg_object(this).m_blue=t_blue;
	pop_err();
}
function c_iLayerObject(){
	c_iGraph.call(this);
	this.m_worldHeight=.0;
	this.m_worldScaleX=.0;
	this.m_worldScaleY=.0;
	this.m_worldRotation=.0;
	this.m_worldWidth=.0;
	this.m_worldX=.0;
	this.m_worldY=.0;
	this.m_parent=null;
	this.m_layer=c_iLayerObject.m_AutoLayer();
	this.m_scoreCollector=null;
	this.m_cosine=.0;
	this.m_cosineRadius=.0;
	this.m_sine=.0;
	this.m_sineRadius=.0;
	this.m_stamina=1;
	this.m_invincible=false;
	this.m_hits=0;
	this.m_scoreCount=false;
	this.m_scoreBoard=0;
	this.m_points=0;
	this.m_countDown=false;
	this.m_collisionRead=false;
	this.m_collisionWrite=false;
	this.m_column=0;
	this.m_row=0;
	this.m_loaderCache=null;
	this.m_collisionMask=[];
	this.m_control=[];
	this.m_debugInfo=new_string_array(8);
	this.implments={c_IMover:1};
}
c_iLayerObject.prototype=extend_class(c_iGraph);
c_iLayerObject.prototype.p_Render=function(){
	push_err();
	pop_err();
}
c_iLayerObject.prototype.p_UpdateWorldXY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2120>";
	dbg_object(this).m_worldHeight=bb_math_Abs2(this.p_Height());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2121>";
	dbg_object(this).m_worldScaleX=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2122>";
	dbg_object(this).m_worldScaleY=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2123>";
	dbg_object(this).m_worldRotation=this.p_Rotation();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2124>";
	dbg_object(this).m_worldWidth=bb_math_Abs2(this.p_Width());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2125>";
	dbg_object(this).m_worldX=dbg_object(this).m_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2126>";
	dbg_object(this).m_worldY=dbg_object(this).m_y;
	pop_err();
}
c_iLayerObject.prototype.p_Update=function(){
	push_err();
	pop_err();
}
c_iLayerObject.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1390>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_OnDisabled=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1310>";
	pop_err();
	return 0;
}
c_iLayerObject.m_AutoLayer=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2265>";
	if(!((bb_globals_iEnginePointer)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2265>";
		c_iEngine.m_new.call(new c_iEngine);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2266>";
	if(!((dbg_object(bb_globals_iEnginePointer).m_playfieldPointer)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2266>";
		c_iPlayfield.m_new.call(new c_iPlayfield);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2267>";
	if(!((dbg_object(bb_globals_iEnginePointer).m_layerPointer)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2267>";
		c_iLayer.m_new.call(new c_iLayer);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2269>";
	pop_err();
	return dbg_object(bb_globals_iEnginePointer).m_layerPointer;
}
c_iLayerObject.prototype.p_Layer=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<953>";
	if((dbg_object(this).m_parent)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<954>";
		pop_err();
		return dbg_object(dbg_object(this).m_parent).m_layer;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<956>";
		pop_err();
		return dbg_object(this).m_layer;
	}
}
c_iLayerObject.prototype.p_Layer2=function(t_layer){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<962>";
	dbg_object(this).m_layer=t_layer;
	pop_err();
}
c_iLayerObject.prototype.p_Cosine=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<719>";
	pop_err();
	return dbg_object(this).m_cosine;
}
c_iLayerObject.prototype.p_Cosine2=function(t_cosine){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<723>";
	dbg_object(this).m_cosine=t_cosine;
	pop_err();
}
c_iLayerObject.prototype.p_CosineRadius=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<727>";
	pop_err();
	return dbg_object(this).m_cosineRadius;
}
c_iLayerObject.prototype.p_CosineRadius2=function(t_cosineRadius){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<731>";
	dbg_object(this).m_cosineRadius=t_cosineRadius;
	pop_err();
}
c_iLayerObject.prototype.p_RenderX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1613>";
	var t_=this.p_PositionX()+Math.cos((this.p_Cosine())*D2R)*this.p_CosineRadius();
	pop_err();
	return t_;
}
c_iLayerObject.prototype.p_Sine=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1735>";
	pop_err();
	return dbg_object(this).m_sine;
}
c_iLayerObject.prototype.p_Sine2=function(t_sine){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1739>";
	dbg_object(this).m_sine=t_sine;
	pop_err();
}
c_iLayerObject.prototype.p_SineRadius=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1743>";
	pop_err();
	return dbg_object(this).m_sineRadius;
}
c_iLayerObject.prototype.p_SineRadius2=function(t_sineRadius){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1747>";
	dbg_object(this).m_sineRadius=t_sineRadius;
	pop_err();
}
c_iLayerObject.prototype.p_RenderY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1622>";
	var t_=this.p_PositionY()+Math.sin((this.p_Sine())*D2R)*this.p_SineRadius();
	pop_err();
	return t_;
}
c_iLayerObject.prototype.p_CollisionMethod=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<643>";
	if(c_iCollision.m_Rectangles(this.p_RenderX(),this.p_RenderY(),((this.p_Width())|0),((this.p_Height())|0),t_layerObject.p_RenderX(),t_layerObject.p_RenderY(),((t_layerObject.p_Width())|0),((t_layerObject.p_Height())|0),1)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<643>";
		pop_err();
		return 1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<644>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_Collides=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<625>";
	if(dbg_object(this).m_enabled==1 && dbg_object(t_layerObject).m_enabled==1 && t_layerObject!=this && t_layerObject!=dbg_object(this).m_scoreCollector){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<626>";
		var t_=this.p_CollisionMethod(t_layerObject);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<629>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_OnOutro=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1319>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_OnRemove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1367>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<736>";
	c_iGraph.prototype.p_Destroy.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<738>";
	this.m_loaderCache=null;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<740>";
	dbg_object(this).m_collisionMask=[];
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<741>";
	dbg_object(this).m_control=[];
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<742>";
	dbg_object(this).m_debugInfo=[];
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<743>";
	dbg_object(this).m_layer=null;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<744>";
	dbg_object(this).m_parent=null;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<745>";
	dbg_object(this).m_scoreCollector=null;
	pop_err();
}
c_iLayerObject.prototype.p_Remove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1489>";
	if((dbg_object(this).m_layer)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1491>";
		if((dbg_object(this).m_enabled)!=0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1493>";
			if(dbg_object(this).m_countDown){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1493>";
				dbg_object(bb_globals_iEnginePointer).m_countDown=dbg_object(bb_globals_iEnginePointer).m_countDown-1;
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1495>";
			dbg_object(this).m_enabled=0;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1499>";
		if(dbg_object(this).m_tattoo==false){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1501>";
			if(dbg_object(this).m_collisionRead){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1502>";
				dbg_object(bb_globals_iEnginePointer).m_collisionReadList.p_Remove2(this);
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1503>";
				dbg_object(this).m_collisionRead=false;
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1506>";
			if(dbg_object(this).m_collisionWrite){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1507>";
				dbg_object(bb_globals_iEnginePointer).m_collisionWriteList.p_Remove2(this);
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1508>";
				dbg_object(this).m_collisionWrite=false;
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1511>";
			this.p_OnRemove();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1513>";
			dbg_array(dbg_array(dbg_object(dbg_object(this).m_layer).m_objectList,this.m_column)[dbg_index],this.m_row)[dbg_index].p_Remove2(this);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1515>";
			if(dbg_object(this).m_inPool==0){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1515>";
				this.p_Destroy();
			}
		}
	}
	pop_err();
}
c_iLayerObject.prototype.p_ScoreSystem=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1776>";
	var t_s1=this.m_stamina;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1777>";
	var t_s2=dbg_object(t_layerObject).m_stamina;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1779>";
	if(dbg_object(this).m_invincible){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1780>";
		if(dbg_object(t_layerObject).m_enabled==1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1780>";
			dbg_object(t_layerObject).m_stamina=0;
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1782>";
		if(dbg_object(t_layerObject).m_enabled==1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1783>";
			dbg_object(this).m_stamina=dbg_object(this).m_stamina-t_s2;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1784>";
			dbg_object(this).m_hits=dbg_object(this).m_hits+1;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1785>";
			dbg_object(this).m_ghost=true;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1789>";
	if(dbg_object(t_layerObject).m_invincible){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1790>";
		if(dbg_object(this).m_enabled==1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1790>";
			dbg_object(this).m_stamina=0;
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1792>";
		if(dbg_object(this).m_enabled==1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1793>";
			dbg_object(t_layerObject).m_stamina=dbg_object(t_layerObject).m_stamina-t_s1;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1794>";
			dbg_object(t_layerObject).m_hits=dbg_object(t_layerObject).m_hits+1;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1795>";
			dbg_object(t_layerObject).m_ghost=true;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1799>";
	if(dbg_object(this).m_stamina<1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1801>";
		dbg_object(this).m_stamina=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1803>";
		if(dbg_object(this).m_scoreCount){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1803>";
			dbg_object(bb_globals_iEnginePointer).m_scoreCount=dbg_object(bb_globals_iEnginePointer).m_scoreCount+1;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1805>";
		if((dbg_object(t_layerObject).m_scoreCollector)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1806>";
			dbg_object(dbg_object(t_layerObject).m_scoreCollector).m_scoreBoard=dbg_object(dbg_object(t_layerObject).m_scoreCollector).m_scoreBoard+dbg_object(this).m_points;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1808>";
			dbg_object(t_layerObject).m_scoreBoard=dbg_object(t_layerObject).m_scoreBoard+dbg_object(this).m_points;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1811>";
		this.p_OnOutro();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1812>";
		this.p_Remove();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1816>";
	if(dbg_object(t_layerObject).m_stamina<1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1818>";
		dbg_object(t_layerObject).m_stamina=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1820>";
		if(dbg_object(t_layerObject).m_scoreCount){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1820>";
			dbg_object(bb_globals_iEnginePointer).m_scoreCount=dbg_object(bb_globals_iEnginePointer).m_scoreCount+1;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1822>";
		if((dbg_object(this).m_scoreCollector)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1823>";
			dbg_object(dbg_object(this).m_scoreCollector).m_scoreBoard=dbg_object(dbg_object(this).m_scoreCollector).m_scoreBoard+dbg_object(t_layerObject).m_points;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1825>";
			dbg_object(this).m_scoreBoard=dbg_object(this).m_scoreBoard+dbg_object(t_layerObject).m_points;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1828>";
		t_layerObject.p_OnOutro();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1829>";
		t_layerObject.p_Remove();
	}
	pop_err();
}
c_iLayerObject.prototype.p_OnCollision=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1302>";
	this.p_ScoreSystem(t_layerObject);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1303>";
	pop_err();
	return 0;
}
c_iLayerObject.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<13>";
	c_iGraph.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<13>";
	pop_err();
	return this;
}
c_iLayerObject.prototype.p_OnAttach=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1293>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_Attach=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<508>";
	this.p_OnAttach();
	pop_err();
}
c_iLayerObject.prototype.p_AttachLast=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<462>";
	if((dbg_object(this).m_layer)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<463>";
		dbg_array(dbg_array(dbg_object(dbg_object(this).m_layer).m_objectList,dbg_object(this).m_column)[dbg_index],dbg_object(this).m_row)[dbg_index].p_Remove2(this);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<464>";
		dbg_object(this).m_column=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<465>";
		dbg_object(this).m_row=0;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<468>";
	dbg_array(dbg_array(dbg_object(dbg_object(bb_globals_iEnginePointer).m_layerPointer).m_objectList,this.m_column)[dbg_index],this.m_row)[dbg_index].p_AddLast3(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<469>";
	dbg_object(this).m_layer=dbg_object(bb_globals_iEnginePointer).m_layerPointer;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<471>";
	this.p_Attach();
	pop_err();
}
c_iLayerObject.prototype.p_AttachLast4=function(t_layer){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<480>";
	if((dbg_object(this).m_layer)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<481>";
		dbg_array(dbg_array(dbg_object(dbg_object(this).m_layer).m_objectList,dbg_object(this).m_column)[dbg_index],dbg_object(this).m_row)[dbg_index].p_Remove2(this);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<482>";
		dbg_object(this).m_column=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<483>";
		dbg_object(this).m_row=0;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<486>";
	dbg_array(dbg_array(dbg_object(t_layer).m_objectList,dbg_object(this).m_column)[dbg_index],dbg_object(this).m_row)[dbg_index].p_AddLast3(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<487>";
	dbg_object(this).m_layer=t_layer;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<489>";
	this.p_Attach();
	pop_err();
}
c_iLayerObject.prototype.p_Parent=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<872>";
	pop_err();
	return dbg_object(this).m_parent;
}
c_iLayerObject.prototype.p_Parent2=function(t_parent){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<876>";
	dbg_object(this).m_parent=t_parent;
	pop_err();
}
c_iLayerObject.prototype.p_AttachLast5=function(t_layerGroup){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<498>";
	t_layerGroup.p_AddItemLast(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<500>";
	this.p_Attach();
	pop_err();
}
c_iLayerObject.prototype.p_Position=function(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1426>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1427>";
	dbg_object(this).m_y=t_y;
	pop_err();
}
c_iLayerObject.prototype.p_Position3=function(t_x,t_y,t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1436>";
	dbg_object(this).m_x=dbg_object(t_layerObject).m_x+t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1437>";
	dbg_object(this).m_y=dbg_object(t_layerObject).m_y+t_y;
	pop_err();
}
c_iLayerObject.prototype.p_Position2=function(t_x,t_y,t_z){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1446>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1447>";
	dbg_object(this).m_y=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1448>";
	dbg_object(this).m_z=t_z;
	pop_err();
}
c_iLayerObject.prototype.p_Position4=function(t_x,t_y,t_z,t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1457>";
	dbg_object(this).m_x=t_layerObject.p_PositionX()+t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1458>";
	dbg_object(this).m_y=t_layerObject.p_PositionY()+t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1459>";
	dbg_object(this).m_z=t_layerObject.p_PositionZ()+t_z;
	pop_err();
}
c_iLayerObject.prototype.p_Show=function(t_info){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1762>";
	for(var t_i=0;t_i<dbg_object(this).m_debugInfo.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1763>";
		if(!((dbg_array(dbg_object(this).m_debugInfo,t_i)[dbg_index]).length!=0)){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1764>";
			dbg_array(dbg_object(this).m_debugInfo,t_i)[dbg_index]=t_info;
			pop_err();
			return;
		}
	}
	pop_err();
}
c_iLayerObject.prototype.p_SetAlpha=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1714>";
	bb_graphics_SetAlpha(t_alpha*dbg_object(bb_globals_iEnginePointer).m_alphaFade*dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_alphaFade*dbg_object(dbg_object(this).m_layer).m_alphaFade);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1716>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_SetColor=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1728>";
	bb_graphics_SetColor(t_red*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_colorFade*dbg_object(dbg_object(this).m_layer).m_colorFade,t_green*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_colorFade*dbg_object(dbg_object(this).m_layer).m_colorFade,t_blue*dbg_object(bb_globals_iEnginePointer).m_colorFade*dbg_object(dbg_object(bb_globals_iEnginePointer).m_currentPlayfield).m_colorFade*dbg_object(dbg_object(this).m_layer).m_colorFade);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1730>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_RenderObject=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1583>";
	dbg_object(t_layerObject).m_layer=dbg_object(this).m_layer;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1585>";
	if(dbg_object(t_layerObject).m_enabled==1 && dbg_object(t_layerObject).m_visible){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1587>";
		bb_graphics_PushMatrix();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1589>";
		bb_graphics_Translate(this.p_PositionX(),this.p_PositionY());
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1590>";
		bb_graphics_Rotate(this.p_Rotation()*(bb_globals_iRotation));
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1591>";
		bb_graphics_Scale(this.p_ScaleX(),this.p_ScaleY());
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1593>";
		var t_alpha=dbg_object(t_layerObject).m_alpha;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1595>";
		dbg_object(t_layerObject).m_alpha=dbg_object(t_layerObject).m_alpha*dbg_object(this).m_alpha;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1597>";
		t_layerObject.p_Render();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1599>";
		dbg_object(t_layerObject).m_alpha=t_alpha;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1601>";
		bb_graphics_PopMatrix();
	}
	pop_err();
}
c_iLayerObject.prototype.p_RenderList=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1566>";
	var t_o=t_list.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1568>";
	while((t_o)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1570>";
		this.p_RenderObject(t_o);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1572>";
		t_o=t_list.p_Ascend();
	}
	pop_err();
}
c_iLayerObject.prototype.p_TestForRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1896>";
	if(dbg_object(this).m_scaleX==0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1896>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1897>";
	if(dbg_object(this).m_scaleY==0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1897>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1898>";
	if(dbg_object(this).m_alpha==0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1898>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<1900>";
	pop_err();
	return true;
}
c_iLayerObject.prototype.p_UpdateObject=function(t_LayerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2040>";
	dbg_object(t_LayerObject).m_layer=dbg_object(this).m_layer;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2042>";
	if(this.p_TestForRender()){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2044>";
		dbg_object(t_LayerObject).m_x=dbg_object(t_LayerObject).m_x*this.p_ScaleX();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2045>";
		dbg_object(t_LayerObject).m_y=dbg_object(t_LayerObject).m_y*this.p_ScaleY();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2047>";
		dbg_object(t_LayerObject).m_x=dbg_object(t_LayerObject).m_x+this.p_RenderX();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2048>";
		dbg_object(t_LayerObject).m_y=dbg_object(t_LayerObject).m_y+this.p_RenderY();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2050>";
		dbg_object(t_LayerObject).m_rotation=dbg_object(t_LayerObject).m_rotation+dbg_object(this).m_rotation;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2052>";
		dbg_object(t_LayerObject).m_scaleX=dbg_object(t_LayerObject).m_scaleX*dbg_object(this).m_scaleX;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2053>";
		dbg_object(t_LayerObject).m_scaleY=dbg_object(t_LayerObject).m_scaleY*dbg_object(this).m_scaleY;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2054>";
		dbg_object(t_LayerObject).m_alpha=dbg_object(t_LayerObject).m_alpha*dbg_object(this).m_alpha;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2056>";
		if(dbg_object(this).m_rotation==0.0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2058>";
			if(dbg_object(t_LayerObject).m_enabled==1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2060>";
				t_LayerObject.p_Update();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2061>";
				t_LayerObject.p_OnUpdate();
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2065>";
				t_LayerObject.p_OnDisabled();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2067>";
				if(dbg_object(t_LayerObject).m_enabled>1){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2067>";
					dbg_object(t_LayerObject).m_enabled=dbg_object(t_LayerObject).m_enabled-1;
				}
			}
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2073>";
			var t_a=(Math.atan2(dbg_object(t_LayerObject).m_y-dbg_object(this).m_y,dbg_object(t_LayerObject).m_x-dbg_object(this).m_x)*R2D)+dbg_object(this).m_rotation*(-bb_globals_iRotation);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2074>";
			var t_d=Math.sqrt(Math.pow(dbg_object(t_LayerObject).m_y-dbg_object(this).m_y,2.0)+Math.pow(dbg_object(t_LayerObject).m_x-dbg_object(this).m_x,2.0));
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2075>";
			var t_tx=dbg_object(this).m_x+Math.cos((t_a)*D2R)*t_d;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2076>";
			var t_ty=dbg_object(this).m_y+Math.sin((t_a)*D2R)*t_d;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2077>";
			t_tx=t_tx-dbg_object(t_LayerObject).m_x;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2078>";
			t_ty=t_ty-dbg_object(t_LayerObject).m_y;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2079>";
			dbg_object(t_LayerObject).m_x=dbg_object(t_LayerObject).m_x+t_tx;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2080>";
			dbg_object(t_LayerObject).m_y=dbg_object(t_LayerObject).m_y+t_ty;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2082>";
			if(dbg_object(t_LayerObject).m_enabled==1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2084>";
				t_LayerObject.p_Update();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2085>";
				t_LayerObject.p_OnUpdate();
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2089>";
				t_LayerObject.p_OnDisabled();
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2091>";
				if(dbg_object(t_LayerObject).m_enabled>1){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2091>";
					dbg_object(t_LayerObject).m_enabled=dbg_object(t_LayerObject).m_enabled-1;
				}
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2095>";
			dbg_object(t_LayerObject).m_x=dbg_object(t_LayerObject).m_x-t_tx;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2096>";
			dbg_object(t_LayerObject).m_y=dbg_object(t_LayerObject).m_y-t_ty;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2100>";
		dbg_object(t_LayerObject).m_x=dbg_object(t_LayerObject).m_x-this.p_RenderX();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2101>";
		dbg_object(t_LayerObject).m_y=dbg_object(t_LayerObject).m_y-this.p_RenderY();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2103>";
		dbg_object(t_LayerObject).m_x=dbg_object(t_LayerObject).m_x/this.p_ScaleX();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2104>";
		dbg_object(t_LayerObject).m_y=dbg_object(t_LayerObject).m_y/this.p_ScaleY();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2106>";
		dbg_object(t_LayerObject).m_rotation=dbg_object(t_LayerObject).m_rotation-dbg_object(this).m_rotation;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2108>";
		dbg_object(t_LayerObject).m_scaleX=dbg_object(t_LayerObject).m_scaleX/dbg_object(this).m_scaleX;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2109>";
		dbg_object(t_LayerObject).m_scaleY=dbg_object(t_LayerObject).m_scaleY/dbg_object(this).m_scaleY;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2110>";
		dbg_object(t_LayerObject).m_alpha=dbg_object(t_LayerObject).m_alpha/dbg_object(this).m_alpha;
	}
	pop_err();
}
c_iLayerObject.prototype.p_UpdateList=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2024>";
	var t_o=t_list.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2025>";
	while((t_o)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2027>";
		this.p_UpdateObject(t_o);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2029>";
		t_o=t_list.p_Ascend();
	}
	pop_err();
}
c_iLayerObject.prototype.p_UpdateWorldXYObject=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2150>";
	var t_x=dbg_object(t_layerObject).m_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2151>";
	var t_y=dbg_object(t_layerObject).m_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2152>";
	var t_rotation=dbg_object(t_layerObject).m_rotation;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2153>";
	var t_scaleX=dbg_object(t_layerObject).m_scaleX;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2154>";
	var t_scaleY=dbg_object(t_layerObject).m_scaleY;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2156>";
	dbg_object(t_layerObject).m_x=dbg_object(t_layerObject).m_x*this.p_ScaleX();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2157>";
	dbg_object(t_layerObject).m_y=dbg_object(t_layerObject).m_y*this.p_ScaleY();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2158>";
	dbg_object(t_layerObject).m_x=dbg_object(t_layerObject).m_x+this.p_PositionX();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2159>";
	dbg_object(t_layerObject).m_y=dbg_object(t_layerObject).m_y+this.p_PositionY();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2160>";
	dbg_object(t_layerObject).m_rotation=dbg_object(t_layerObject).m_rotation+dbg_object(this).m_rotation;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2161>";
	dbg_object(t_layerObject).m_scaleX=dbg_object(t_layerObject).m_scaleX*dbg_object(this).m_scaleX;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2162>";
	dbg_object(t_layerObject).m_scaleY=dbg_object(t_layerObject).m_scaleY*dbg_object(this).m_scaleY;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2164>";
	var t_a=(Math.atan2(dbg_object(t_layerObject).m_y-dbg_object(this).m_y,dbg_object(t_layerObject).m_x-dbg_object(this).m_x)*R2D)+dbg_object(this).m_rotation*(-bb_globals_iRotation);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2165>";
	var t_d=Math.sqrt(Math.pow(dbg_object(t_layerObject).m_y-dbg_object(this).m_y,2.0)+Math.pow(dbg_object(t_layerObject).m_x-dbg_object(this).m_x,2.0));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2166>";
	var t_tx=dbg_object(this).m_x+Math.cos((t_a)*D2R)*t_d;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2167>";
	var t_ty=dbg_object(this).m_y+Math.sin((t_a)*D2R)*t_d;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2168>";
	t_tx=t_tx-dbg_object(t_layerObject).m_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2169>";
	t_ty=t_ty-dbg_object(t_layerObject).m_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2170>";
	dbg_object(t_layerObject).m_x=dbg_object(t_layerObject).m_x+t_tx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2171>";
	dbg_object(t_layerObject).m_y=dbg_object(t_layerObject).m_y+t_ty;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2173>";
	t_layerObject.p_UpdateWorldXY();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2175>";
	dbg_object(t_layerObject).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2176>";
	dbg_object(t_layerObject).m_y=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2177>";
	dbg_object(t_layerObject).m_rotation=t_rotation;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2178>";
	dbg_object(t_layerObject).m_scaleX=t_scaleX;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2179>";
	dbg_object(t_layerObject).m_scaleY=t_scaleY;
	pop_err();
}
c_iLayerObject.prototype.p_UpdateWorldXYList=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2134>";
	var t_o=t_list.p_First();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2135>";
	while((t_o)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2137>";
		this.p_UpdateWorldXYObject(t_o);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layerobject.monkey<2139>";
		t_o=t_list.p_Ascend();
	}
	pop_err();
}
function c_iList3(){
	Object.call(this);
	this.m_index=0;
	this.m_length=0;
	this.m_data=[];
}
c_iList3.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<97>";
	dbg_object(this).m_index=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
	if((dbg_object(this).m_length)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList3.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<67>";
	dbg_object(this).m_index=dbg_object(this).m_index+1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
	if(dbg_object(this).m_index>=0 && dbg_object(this).m_index<dbg_object(this).m_length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,dbg_object(this).m_index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList3.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList3.prototype.p_Position5=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<161>";
	for(var t_i=0;t_i<dbg_object(this).m_length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<162>";
		if(dbg_array(dbg_object(this).m_data,t_i)[dbg_index]==t_value){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<162>";
			pop_err();
			return t_i;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<164>";
	pop_err();
	return -1;
}
c_iList3.prototype.p_RemoveFromIndex=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<186>";
	for(var t_i=t_index;t_i<dbg_object(this).m_length-1;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<187>";
		dbg_array(dbg_object(this).m_data,t_i)[dbg_index]=dbg_array(dbg_object(this).m_data,t_i+1)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<190>";
	dbg_object(this).m_length=dbg_object(this).m_length-1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<191>";
	dbg_object(this).m_index=dbg_object(this).m_index-1;
	pop_err();
}
c_iList3.prototype.p_Remove2=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<174>";
	var t_p=this.p_Position5(t_value);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<175>";
	if(t_p>=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<175>";
		this.p_RemoveFromIndex(t_p);
	}
	pop_err();
}
c_iList3.prototype.p_AddLast3=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<55>";
	if(dbg_object(this).m_length==dbg_object(this).m_data.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<56>";
		dbg_object(this).m_data=resize_object_array(dbg_object(this).m_data,dbg_object(this).m_length*2+10);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<58>";
	dbg_array(dbg_object(this).m_data,dbg_object(this).m_length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<59>";
	dbg_object(this).m_length=dbg_object(this).m_length+1;
	pop_err();
}
function c_iGuiObject(){
	c_iGraph.call(this);
	this.m_worldHeight=.0;
	this.m_worldScaleX=.0;
	this.m_worldScaleY=.0;
	this.m_worldRotation=.0;
	this.m_worldWidth=.0;
	this.m_worldX=.0;
	this.m_worldY=.0;
	this.m_multiTouch=false;
	this.m_touchIndex=0;
	this.m_parent=null;
	this.m_playfield=c_iGuiObject.m_AutoPlayfield();
	this.m_handleX=0;
	this.m_handleY=0;
	this.m_touchDown=0;
	this.m_over=false;
	this.m_rememberTopObject=true;
	this.implments={c_IMover:1};
}
c_iGuiObject.prototype=extend_class(c_iGraph);
c_iGuiObject.prototype.p_RenderGhost=function(){
	push_err();
	pop_err();
}
c_iGuiObject.prototype.p_Render=function(){
	push_err();
	pop_err();
}
c_iGuiObject.m_topObject=null;
c_iGuiObject.prototype.p_UpdateWorldXY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1688>";
	dbg_object(this).m_worldHeight=bb_math_Abs2(this.p_Height());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1689>";
	dbg_object(this).m_worldScaleX=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1690>";
	dbg_object(this).m_worldScaleY=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1691>";
	dbg_object(this).m_worldRotation=this.p_Rotation();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1692>";
	dbg_object(this).m_worldWidth=bb_math_Abs2(this.p_Width());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1693>";
	dbg_object(this).m_worldX=dbg_object(this).m_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1694>";
	dbg_object(this).m_worldY=dbg_object(this).m_y;
	pop_err();
}
c_iGuiObject.prototype.p_Parent=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<994>";
	pop_err();
	return dbg_object(this).m_parent;
}
c_iGuiObject.prototype.p_Parent3=function(t_parent){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<998>";
	dbg_object(this).m_parent=t_parent;
	pop_err();
}
c_iGuiObject.m_AutoPlayfield=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1833>";
	if(!((bb_globals_iEnginePointer)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1833>";
		c_iEngine.m_new.call(new c_iEngine);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1834>";
	if(!((dbg_object(bb_globals_iEnginePointer).m_playfieldPointer)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1834>";
		c_iPlayfield.m_new.call(new c_iPlayfield);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1836>";
	pop_err();
	return dbg_object(bb_globals_iEnginePointer).m_playfieldPointer;
}
c_iGuiObject.prototype.p_Playfield=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1127>";
	if((this.p_Parent())!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1128>";
		var t_=dbg_object(this.p_Parent()).m_playfield;
		pop_err();
		return t_;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1130>";
		pop_err();
		return dbg_object(this).m_playfield;
	}
}
c_iGuiObject.prototype.p_Playfield2=function(t_playfield){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1136>";
	dbg_object(this).m_playfield=t_playfield;
	pop_err();
}
c_iGuiObject.prototype.p_TouchX=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1456>";
	if(!((bb_globals_iEnginePointer)!=null) || !((this.p_Playfield())!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1456>";
		var t_=bb_input_TouchX(t_index);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1458>";
	var t_2=bb_input_TouchX(t_index)/dbg_object(this.p_Playfield()).m_scaleX-dbg_object(this.p_Playfield()).m_x/dbg_object(this.p_Playfield()).m_scaleX;
	pop_err();
	return t_2;
}
c_iGuiObject.prototype.p_TouchY=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1464>";
	if(!((bb_globals_iEnginePointer)!=null) || !((this.p_Playfield())!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1464>";
		var t_=bb_input_TouchY(t_index);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1466>";
	var t_2=bb_input_TouchY(t_index)/dbg_object(this.p_Playfield()).m_scaleY-dbg_object(this.p_Playfield()).m_y/dbg_object(this.p_Playfield()).m_scaleY;
	pop_err();
	return t_2;
}
c_iGuiObject.prototype.p_TouchOver=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1446>";
	if(this.p_Playfield()!=(c_iSystemGui.m_playfield) && this.p_Playfield()!=bb_functions_iGetTouchPlayfield(t_index)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1446>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1448>";
	if(bb_commoncode_iPointInsideRectange(dbg_object(this).m_worldX,dbg_object(this).m_worldY,dbg_object(this).m_worldWidth,dbg_object(this).m_worldHeight,this.p_TouchX(t_index),this.p_TouchY(t_index),dbg_object(this).m_worldRotation*(bb_globals_iRotation),(dbg_object(this).m_handleX)*bb_math_Abs2(dbg_object(this).m_worldScaleX),(dbg_object(this).m_handleY)*bb_math_Abs2(dbg_object(this).m_worldScaleY))){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1448>";
		pop_err();
		return true;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1450>";
	pop_err();
	return false;
}
c_iGuiObject.prototype.p_MultiTouch=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<880>";
	for(this.m_touchIndex=0;this.m_touchIndex<bb_functions_iTouchDowns();this.m_touchIndex=this.m_touchIndex+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<881>";
		if(((bb_input_TouchDown(dbg_object(this).m_touchIndex))!=0) && this.p_TouchOver(dbg_object(this).m_touchIndex)){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<881>";
			pop_err();
			return true;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<884>";
	pop_err();
	return false;
}
c_iGuiObject.prototype.p_UpdateInput=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1615>";
	if(dbg_object(this).m_multiTouch){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1615>";
		this.p_MultiTouch();
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1617>";
	dbg_object(this).m_touchDown=bb_input_TouchDown(dbg_object(this).m_touchIndex);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1619>";
	dbg_object(this).m_over=this.p_TouchOver(dbg_object(this).m_touchIndex);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1621>";
	if(dbg_object(this).m_rememberTopObject){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1623>";
		if(!dbg_object(this).m_multiTouch && ((c_iGuiObject.m_topObject)!=null)){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1623>";
			this.m_over=false;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1625>";
		if(!((c_iGuiObject.m_topObject)!=null) && dbg_object(this).m_over){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<1625>";
			c_iGuiObject.m_topObject=this;
		}
	}
	pop_err();
}
c_iGuiObject.prototype.p_UpdateGhost=function(){
	push_err();
	pop_err();
}
c_iGuiObject.prototype.p_Update=function(){
	push_err();
	pop_err();
}
c_iGuiObject.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<974>";
	pop_err();
	return 0;
}
c_iGuiObject.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<650>";
	c_iGraph.prototype.p_Destroy.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<652>";
	dbg_object(this).m_parent=null;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/guiobject.monkey<653>";
	dbg_object(this).m_playfield=null;
	pop_err();
}
function c_iList4(){
	Object.call(this);
	this.m_index=0;
	this.m_length=0;
	this.m_data=[];
}
c_iList4.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<97>";
	dbg_object(this).m_index=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
	if((dbg_object(this).m_length)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList4.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<67>";
	dbg_object(this).m_index=dbg_object(this).m_index+1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
	if(dbg_object(this).m_index>=0 && dbg_object(this).m_index<dbg_object(this).m_length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,dbg_object(this).m_index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList4.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList4.prototype.p_Last=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<138>";
	dbg_object(this).m_index=dbg_object(this).m_length-1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<139>";
	if((dbg_object(this).m_length)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<139>";
		var t_=dbg_array(dbg_object(this).m_data,dbg_object(this).m_length-1)[dbg_index];
		pop_err();
		return t_;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<139>";
		pop_err();
		return null;
	}
}
c_iList4.prototype.p_Descend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<87>";
	dbg_object(this).m_index=dbg_object(this).m_index-1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<88>";
	if(dbg_object(this).m_index>=0 && dbg_object(this).m_index<dbg_object(this).m_length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<88>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,dbg_object(this).m_index)[dbg_index];
	}
	pop_err();
	return null;
}
function c_iSystemGui(){
	c_iPlayfield.call(this);
}
c_iSystemGui.prototype=extend_class(c_iPlayfield);
c_iSystemGui.m_playfield=null;
c_iSystemGui.m_Playfield2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/systemgui.monkey<176>";
	var t_=(c_iSystemGui.m_playfield);
	pop_err();
	return t_;
}
c_iSystemGui.m_GuiPage=function(t_page){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/systemgui.monkey<70>";
	if((c_iSystemGui.m_Playfield2())!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/systemgui.monkey<70>";
		c_iSystemGui.m_Playfield2().p_GuiPage2(t_page);
	}
	pop_err();
}
c_iSystemGui.prototype.p_Set2=function(){
	push_err();
	pop_err();
}
c_iSystemGui.prototype.p_SetAlpha=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/systemgui.monkey<186>";
	bb_graphics_SetAlpha(t_alpha);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/systemgui.monkey<187>";
	pop_err();
	return 0;
}
c_iSystemGui.prototype.p_SetColor=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/systemgui.monkey<194>";
	bb_graphics_SetColor(t_red,t_green,t_blue);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/systemgui.monkey<195>";
	pop_err();
	return 0;
}
function c_iTask(){
	c_iObject.call(this);
	this.m_visible=1;
	this.m_cold=true;
	this.m_started=false;
}
c_iTask.prototype=extend_class(c_iObject);
c_iTask.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<73>";
	pop_err();
	return 0;
}
c_iTask.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<177>";
	this.p_OnRender();
	pop_err();
}
c_iTask.m_Render=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<185>";
	if((t_list)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<187>";
		if((t_list.p_Length())!=0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<189>";
			var t_t=t_list.p_First();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<190>";
			while((t_t)!=null){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<192>";
				if((dbg_object(t_t).m_enabled)!=0){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<194>";
					if(dbg_object(t_t).m_visible==1){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<195>";
						t_t.p_Render();
					}else{
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<197>";
						if(dbg_object(t_t).m_visible>1){
							err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<197>";
							dbg_object(t_t).m_visible=dbg_object(t_t).m_visible-1;
						}
					}
				}
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<202>";
				t_t=t_list.p_Ascend();
			}
		}
	}
	pop_err();
}
c_iTask.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<80>";
	pop_err();
	return 0;
}
c_iTask.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<230>";
	this.p_OnUpdate();
	pop_err();
}
c_iTask.prototype.p_SystemStart=function(){
	push_err();
	pop_err();
}
c_iTask.prototype.p_OnColdStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<43>";
	pop_err();
	return 0;
}
c_iTask.prototype.p_OnStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<59>";
	pop_err();
	return 0;
}
c_iTask.m_Update=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<238>";
	if((t_list)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<240>";
		if((t_list.p_Length())!=0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<242>";
			var t_t=t_list.p_First();
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<244>";
			while((t_t)!=null){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<246>";
				if(dbg_object(t_t).m_enabled==1){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<248>";
					if(dbg_object(t_t).m_cold==true || dbg_object(t_t).m_started==false){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<248>";
						t_t.p_SystemStart();
					}
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<250>";
					if(dbg_object(t_t).m_cold==true){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<251>";
						t_t.p_OnColdStart();
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<252>";
						dbg_object(t_t).m_cold=false;
					}
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<255>";
					if(dbg_object(t_t).m_started==false){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<256>";
						t_t.p_OnStart();
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<257>";
						dbg_object(t_t).m_started=true;
					}
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<260>";
					t_t.p_Update();
				}else{
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<264>";
					if(dbg_object(t_t).m_enabled>1){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<264>";
						dbg_object(t_t).m_enabled=dbg_object(t_t).m_enabled-1;
					}
				}
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/task.monkey<268>";
				t_t=t_list.p_Ascend();
			}
		}
	}
	pop_err();
}
function c_iList5(){
	Object.call(this);
	this.m_length=0;
	this.m_index=0;
	this.m_data=[];
}
c_iList5.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList5.prototype.p_Length=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<147>";
	pop_err();
	return dbg_object(this).m_length;
}
c_iList5.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<97>";
	dbg_object(this).m_index=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
	if((dbg_object(this).m_length)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<98>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList5.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<67>";
	dbg_object(this).m_index=dbg_object(this).m_index+1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
	if(dbg_object(this).m_index>=0 && dbg_object(this).m_index<dbg_object(this).m_length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/list.monkey<68>";
		pop_err();
		return dbg_array(dbg_object(this).m_data,dbg_object(this).m_index)[dbg_index];
	}
	pop_err();
	return null;
}
var bb_globals_iTaskList=null;
function bb_math_Abs(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/math.monkey<46>";
	if(t_x>=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/math.monkey<46>";
		pop_err();
		return t_x;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/math.monkey<47>";
	var t_=-t_x;
	pop_err();
	return t_;
}
function bb_math_Abs2(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/math.monkey<73>";
	if(t_x>=0.0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/math.monkey<73>";
		pop_err();
		return t_x;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/math.monkey<74>";
	var t_=-t_x;
	pop_err();
	return t_;
}
var bb_globals_iMultiTouch=0;
function bb_input_TouchDown(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/input.monkey<84>";
	var t_=((bb_input_device.p_KeyDown(384+t_index))?1:0);
	pop_err();
	return t_;
}
function bb_functions_iTouchDowns(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<420>";
	var t_i=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<422>";
	for(t_i=0;t_i<bb_globals_iMultiTouch;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<423>";
		if((bb_input_TouchDown(t_i))!=0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<423>";
			t_i=t_i+1;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<426>";
	pop_err();
	return t_i;
}
function bb_input_TouchX(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/input.monkey<76>";
	var t_=bb_input_device.p_TouchX(t_index);
	pop_err();
	return t_;
}
function bb_input_TouchY(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/input.monkey<80>";
	var t_=bb_input_device.p_TouchY(t_index);
	pop_err();
	return t_;
}
function bb_functions_iGetTouchPlayfield(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<137>";
	if(dbg_object(bb_globals_iEnginePointer).m_playfieldList.p_Length()==0 && ((dbg_object(bb_globals_iEnginePointer).m_playfieldPointer)!=null) && ((dbg_object(dbg_object(bb_globals_iEnginePointer).m_playfieldPointer).m_enabled)!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<137>";
		pop_err();
		return dbg_object(bb_globals_iEnginePointer).m_playfieldPointer;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<139>";
	if(dbg_object(bb_globals_iEnginePointer).m_playfieldList.p_Length()==1 && ((dbg_object(dbg_object(bb_globals_iEnginePointer).m_playfieldList.p_First()).m_enabled)!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<141>";
		var t_p=dbg_object(bb_globals_iEnginePointer).m_playfieldList.p_First();
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<143>";
		if(bb_input_TouchX(t_index)>=dbg_object(t_p).m_x && bb_input_TouchY(t_index)>=dbg_object(t_p).m_y && bb_input_TouchX(t_index)<=dbg_object(t_p).m_x+(dbg_object(t_p).m_width)*dbg_object(t_p).m_scaleX && bb_input_TouchY(t_index)<=dbg_object(t_p).m_y+(dbg_object(t_p).m_height)*dbg_object(t_p).m_scaleY){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<143>";
			pop_err();
			return t_p;
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<147>";
		for(var t_i=dbg_object(bb_globals_iEnginePointer).m_playfieldList.p_Length()-1;t_i>=0;t_i=t_i+-1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<149>";
			var t_p2=dbg_object(bb_globals_iEnginePointer).m_playfieldList.p_Get(t_i);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<151>";
			if(bb_input_TouchX(t_index)>=dbg_object(t_p2).m_x && bb_input_TouchY(t_index)>=dbg_object(t_p2).m_y && bb_input_TouchX(t_index)<=dbg_object(t_p2).m_x+(dbg_object(t_p2).m_width)*dbg_object(t_p2).m_scaleX && bb_input_TouchY(t_index)<=dbg_object(t_p2).m_y+(dbg_object(t_p2).m_height)*dbg_object(t_p2).m_scaleY){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<151>";
				pop_err();
				return t_p2;
			}
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/functions.monkey<157>";
	pop_err();
	return null;
}
function bb_commoncode_iPointInsideRectange(t_x,t_y,t_width,t_height,t_pointX,t_pointY,t_rotation,t_handleX,t_handleY){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<92>";
	var t_c=Math.cos((t_rotation)*D2R);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<93>";
	var t_s=Math.sin((t_rotation)*D2R);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<95>";
	var t_rotatedX=t_x+t_c*(t_pointX-t_x)-t_s*(t_pointY-t_y)+t_handleX;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<96>";
	var t_rotatedY=t_y+t_s*(t_pointX-t_x)+t_c*(t_pointY-t_y)+t_handleY;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<98>";
	var t_leftX=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<99>";
	var t_rightX=t_x+t_width;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<100>";
	var t_topY=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<101>";
	var t_bottomY=t_y+t_height;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/commoncode.monkey<103>";
	var t_=t_leftX<=t_rotatedX && t_rotatedX<=t_rightX && t_topY<=t_rotatedY && t_rotatedY<=t_bottomY;
	pop_err();
	return t_;
}
function c_iCollision(){
	Object.call(this);
}
c_iCollision.m_Rectangles=function(t_x1,t_y1,t_width1,t_height1,t_x2,t_y2,t_width2,t_height2,t_centerRect){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<137>";
	if((t_centerRect)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<138>";
		t_x1=t_x1-((t_width1/2)|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<139>";
		t_y1=t_y1-((t_height1/2)|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<140>";
		t_x2=t_x2-((t_width2/2)|0);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<141>";
		t_y2=t_y2-((t_height2/2)|0);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<144>";
	if(t_x1+(t_width1)<=t_x2){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<144>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<145>";
	if(t_y1+(t_height1)<=t_y2){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<145>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<147>";
	if(t_x1>=t_x2+(t_width2)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<147>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<148>";
	if(t_y1>=t_y2+(t_height2)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<148>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/collision.monkey<150>";
	pop_err();
	return true;
}
function c_iVector2d(){
	Object.call(this);
	this.m_x=0.0;
	this.m_y=0.0;
}
c_iVector2d.m_new=function(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/vector2d.monkey<31>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/vector2d.monkey<32>";
	dbg_object(this).m_y=t_y;
	pop_err();
	return this;
}
c_iVector2d.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/math/vector2d.monkey<15>";
	pop_err();
	return this;
}
function c_iConfig(){
	Object.call(this);
}
function c_iControlSet(){
	Object.call(this);
}
function bb_graphics_DrawImage(t_image,t_x,t_y,t_frame){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<452>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<453>";
	if(t_frame<0 || t_frame>=dbg_object(t_image).m_frames.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<453>";
		error("Invalid image frame");
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<456>";
	var t_f=dbg_array(dbg_object(t_image).m_frames,t_frame)[dbg_index];
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<458>";
	bb_graphics_context.p_Validate();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<460>";
	if((dbg_object(t_image).m_flags&65536)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<461>";
		bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).m_surface,t_x-dbg_object(t_image).m_tx,t_y-dbg_object(t_image).m_ty);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<463>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).m_surface,t_x-dbg_object(t_image).m_tx,t_y-dbg_object(t_image).m_ty,dbg_object(t_f).m_x,dbg_object(t_f).m_y,dbg_object(t_image).m_width,dbg_object(t_image).m_height);
	}
	pop_err();
	return 0;
}
function bb_graphics_PushMatrix(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<333>";
	var t_sp=dbg_object(bb_graphics_context).m_matrixSp;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<334>";
	if(t_sp==dbg_object(bb_graphics_context).m_matrixStack.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<334>";
		dbg_object(bb_graphics_context).m_matrixStack=resize_number_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp*2);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<335>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+0)[dbg_index]=dbg_object(bb_graphics_context).m_ix;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<336>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+1)[dbg_index]=dbg_object(bb_graphics_context).m_iy;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<337>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+2)[dbg_index]=dbg_object(bb_graphics_context).m_jx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<338>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+3)[dbg_index]=dbg_object(bb_graphics_context).m_jy;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<339>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+4)[dbg_index]=dbg_object(bb_graphics_context).m_tx;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<340>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+5)[dbg_index]=dbg_object(bb_graphics_context).m_ty;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<341>";
	dbg_object(bb_graphics_context).m_matrixSp=t_sp+6;
	pop_err();
	return 0;
}
function bb_graphics_PopMatrix(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<345>";
	var t_sp=dbg_object(bb_graphics_context).m_matrixSp-6;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<346>";
	bb_graphics_SetMatrix(dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+0)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+1)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+2)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+3)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+4)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+5)[dbg_index]);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<347>";
	dbg_object(bb_graphics_context).m_matrixSp=t_sp;
	pop_err();
	return 0;
}
function bb_graphics_DrawImage2(t_image,t_x,t_y,t_rotation,t_scaleX,t_scaleY,t_frame){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<470>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<471>";
	if(t_frame<0 || t_frame>=dbg_object(t_image).m_frames.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<471>";
		error("Invalid image frame");
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<474>";
	var t_f=dbg_array(dbg_object(t_image).m_frames,t_frame)[dbg_index];
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<476>";
	bb_graphics_PushMatrix();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<478>";
	bb_graphics_Translate(t_x,t_y);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<479>";
	bb_graphics_Rotate(t_rotation);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<480>";
	bb_graphics_Scale(t_scaleX,t_scaleY);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<482>";
	bb_graphics_Translate(-dbg_object(t_image).m_tx,-dbg_object(t_image).m_ty);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<484>";
	bb_graphics_context.p_Validate();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<486>";
	if((dbg_object(t_image).m_flags&65536)!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<487>";
		bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).m_surface,0.0,0.0);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<489>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).m_surface,0.0,0.0,dbg_object(t_f).m_x,dbg_object(t_f).m_y,dbg_object(t_image).m_width,dbg_object(t_image).m_height);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<492>";
	bb_graphics_PopMatrix();
	pop_err();
	return 0;
}
function bb_graphics_DrawText(t_text,t_x,t_y,t_xalign,t_yalign){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<577>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<579>";
	if(!((dbg_object(bb_graphics_context).m_font)!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<579>";
		pop_err();
		return 0;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<581>";
	var t_w=dbg_object(bb_graphics_context).m_font.p_Width();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<582>";
	var t_h=dbg_object(bb_graphics_context).m_font.p_Height();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<584>";
	t_x-=Math.floor((t_w*t_text.length)*t_xalign);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<585>";
	t_y-=Math.floor((t_h)*t_yalign);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<587>";
	for(var t_i=0;t_i<t_text.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<588>";
		var t_ch=dbg_charCodeAt(t_text,t_i)-dbg_object(bb_graphics_context).m_firstChar;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<589>";
		if(t_ch>=0 && t_ch<dbg_object(bb_graphics_context).m_font.p_Frames()){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<590>";
			bb_graphics_DrawImage(dbg_object(bb_graphics_context).m_font,t_x+(t_i*t_w),t_y,t_ch);
		}
	}
	pop_err();
	return 0;
}
function bb_input_KeyHit(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/input.monkey<44>";
	var t_=bb_input_device.p_KeyHit(t_key);
	pop_err();
	return t_;
}
function c_iLayerSprite(){
	c_iLayerObject.call(this);
	this.m_imagePointer=new_object_array(1);
	this.m_imageSignature=new_object_array(1);
	this.m_ghostImagePointer=new_object_array(1);
	this.m_frame=.0;
	this.m_imageIndex=.0;
	this.m_frameOffset=0.25;
	this.m_animationMode=1;
	this.m_imagePath="";
	this.implments={c_IMover:1};
}
c_iLayerSprite.prototype=extend_class(c_iLayerObject);
c_iLayerSprite.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<11>";
	c_iLayerObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<11>";
	pop_err();
	return this;
}
c_iLayerSprite.prototype.p_ImagePointer=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<423>";
	pop_err();
	return dbg_object(this).m_imagePointer;
}
c_iLayerSprite.prototype.p_ImagePointer2=function(t_image){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<431>";
	if(t_image!=dbg_array(dbg_object(this).m_imageSignature,0)[dbg_index]){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<433>";
		t_image.p_SetHandle((t_image.p_Width())*.5,(t_image.p_Height())*.5);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<435>";
		dbg_array(dbg_object(this).m_imagePointer,0)[dbg_index]=t_image;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<436>";
		dbg_array(dbg_object(this).m_ghostImagePointer,0)[dbg_index]=t_image;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<437>";
		dbg_array(dbg_object(this).m_imageSignature,0)[dbg_index]=t_image;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<439>";
		dbg_object(this).m_frame=0.0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<440>";
		dbg_object(this).m_imageIndex=0.0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<442>";
		dbg_object(this).m_frameOffset=1.0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<443>";
		dbg_object(this).m_animationMode=1;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<445>";
		if(dbg_object(this).m_imagePointer.length>1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<445>";
			dbg_object(this).m_imagePointer=resize_object_array(dbg_object(this).m_imagePointer,1);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<446>";
		if(dbg_object(this).m_ghostImagePointer.length>1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<446>";
			dbg_object(this).m_ghostImagePointer=resize_object_array(dbg_object(this).m_ghostImagePointer,1);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<447>";
		if(dbg_object(this).m_imageSignature.length>1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<447>";
			dbg_object(this).m_imageSignature=resize_object_array(dbg_object(this).m_imageSignature,1);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<449>";
		dbg_object(this).m_imagePath="";
	}
	pop_err();
}
c_iLayerSprite.prototype.p_Compare2=function(t_image1,t_image2){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<208>";
	if(t_image1.length!=t_image2.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<208>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<210>";
	for(var t_i=0;t_i<t_image1.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<211>";
		if(dbg_array(t_image1,t_i)[dbg_index]!=dbg_array(t_image2,t_i)[dbg_index]){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<211>";
			pop_err();
			return false;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<214>";
	pop_err();
	return true;
}
c_iLayerSprite.prototype.p_ImagePointer3=function(t_image){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<460>";
	if(this.p_Compare2(t_image,dbg_object(this).m_imageSignature)==false){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<462>";
		for(var t_i=0;t_i<t_image.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<463>";
			dbg_array(t_image,t_i)[dbg_index].p_SetHandle((dbg_array(t_image,t_i)[dbg_index].p_Width())*.5,(dbg_array(t_image,t_i)[dbg_index].p_Height())*.5);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<466>";
		dbg_object(this).m_imagePointer=resize_object_array(dbg_object(this).m_imagePointer,t_image.length);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<467>";
		for(var t_i2=0;t_i2<t_image.length;t_i2=t_i2+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<468>";
			dbg_array(dbg_object(this).m_imagePointer,t_i2)[dbg_index]=dbg_array(t_image,t_i2)[dbg_index];
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<471>";
		dbg_object(this).m_ghostImagePointer=resize_object_array(dbg_object(this).m_ghostImagePointer,t_image.length);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<472>";
		for(var t_i3=0;t_i3<t_image.length;t_i3=t_i3+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<473>";
			dbg_array(dbg_object(this).m_ghostImagePointer,t_i3)[dbg_index]=dbg_array(t_image,t_i3)[dbg_index];
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<476>";
		dbg_object(this).m_imageSignature=resize_object_array(dbg_object(this).m_imageSignature,t_image.length);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<477>";
		for(var t_i4=0;t_i4<t_image.length;t_i4=t_i4+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<478>";
			dbg_array(dbg_object(this).m_imageSignature,t_i4)[dbg_index]=dbg_array(t_image,t_i4)[dbg_index];
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<481>";
		dbg_object(this).m_frame=0.0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<482>";
		dbg_object(this).m_imageIndex=0.0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<484>";
		dbg_object(this).m_frameOffset=1.0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<485>";
		dbg_object(this).m_animationMode=1;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<487>";
		dbg_object(this).m_imagePath="";
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImagePointer4=function(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<562>";
	if(t_path!=dbg_object(this).m_imagePath){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<564>";
		var t_image=bb_contentmanager_iContent.p_GetImage4(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,c_Image.m_DefaultFlags);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<566>";
		this.p_ImagePointer3(t_image);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<567>";
		dbg_object(this).m_imagePath=t_path;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImagePointer5=function(t_start,t_count,t_path,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<542>";
	if(t_path!=dbg_object(this).m_imagePath){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<544>";
		var t_image=bb_contentmanager_iContent.p_GetImage3(t_start,t_count,t_path,t_frameCount,c_Image.m_DefaultFlags);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<546>";
		this.p_ImagePointer3(t_image);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<547>";
		dbg_object(this).m_imagePath=t_path;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImagePointer6=function(t_path,t_frameWidth,t_frameHeight,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<522>";
	if(t_path!=dbg_object(this).m_imagePath){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<524>";
		var t_image=bb_contentmanager_iContent.p_GetImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,c_Image.m_DefaultFlags);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<526>";
		this.p_ImagePointer2(t_image);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<527>";
		dbg_object(this).m_imagePath=t_path;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImagePointer7=function(t_path,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<502>";
	if(t_path!=dbg_object(this).m_imagePath){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<504>";
		var t_image=bb_contentmanager_iContent.p_GetImage(t_path,t_frameCount,c_Image.m_DefaultFlags);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<506>";
		this.p_ImagePointer2(t_image);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<507>";
		dbg_object(this).m_imagePath=t_path;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_SetImage=function(t_image){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<712>";
	this.p_ImagePointer2(t_image);
	pop_err();
}
c_iLayerSprite.prototype.p_SetImage2=function(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<720>";
	if(t_path!=dbg_object(this).m_imagePath){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<722>";
		var t_o=bb_contentmanager_iContent.p_Get2(t_path);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<723>";
		if((t_o)!=null){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<725>";
			if((dbg_object(t_o).m_imagePointer)!=null){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<725>";
				this.p_ImagePointer2(dbg_object(t_o).m_imagePointer);
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<726>";
			if((dbg_object(t_o).m_imagePointers.length)!=0){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<726>";
				this.p_ImagePointer3(dbg_object(t_o).m_imagePointers);
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<728>";
			dbg_object(this).m_imagePath=t_path;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<730>";
			dbg_object(this).m_animationMode=1;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<731>";
			dbg_object(this).m_frame=0.0;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<732>";
			dbg_object(this).m_frameOffset=.025;
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<733>";
			dbg_object(this).m_imageIndex=0.0;
		}
	}
	pop_err();
}
c_iLayerSprite.prototype.p_AnimationLoop=function(t_frameOffset,t_path){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<55>";
	if((t_path).length!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<55>";
		this.p_SetImage2(t_path);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<57>";
	var t_finished=false;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<59>";
	if(dbg_object(this).m_imagePointer.length>1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<61>";
		dbg_object(this).m_imageIndex=dbg_object(this).m_imageIndex+t_frameOffset*(dbg_object(this).m_animationMode);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<63>";
		if(dbg_object(this).m_imageIndex<0.0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<64>";
			dbg_object(this).m_imageIndex=dbg_object(this).m_imageIndex+(dbg_object(this).m_imagePointer.length);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<65>";
			t_finished=true;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<68>";
		if(this.m_imageIndex>=(this.m_imagePointer.length)){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<69>";
			dbg_object(this).m_imageIndex=dbg_object(this).m_imageIndex-(dbg_object(this).m_imagePointer.length);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<70>";
			t_finished=true;
		}
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<75>";
		dbg_object(this).m_frame=dbg_object(this).m_frame+t_frameOffset*(dbg_object(this).m_animationMode);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<77>";
		if(dbg_object(this).m_frame<0.0){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<78>";
			dbg_object(this).m_frame=dbg_object(this).m_frame+(dbg_array(dbg_object(this).m_imagePointer,((dbg_object(this).m_imageIndex)|0))[dbg_index].p_Frames());
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<79>";
			t_finished=true;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<82>";
		if(dbg_object(this).m_frame>=(dbg_array(dbg_object(this).m_imagePointer,((dbg_object(this).m_imageIndex)|0))[dbg_index].p_Frames())){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<83>";
			dbg_object(this).m_frame=dbg_object(this).m_frame-(dbg_array(dbg_object(this).m_imagePointer,((dbg_object(this).m_imageIndex)|0))[dbg_index].p_Frames());
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<84>";
			t_finished=true;
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<89>";
	pop_err();
	return t_finished;
}
c_iLayerSprite.prototype.p_Frame=function(t_frame){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<273>";
	if(dbg_object(this).m_imagePointer.length>1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<275>";
		dbg_object(this).m_imageIndex=(t_frame);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<279>";
		dbg_object(this).m_frame=(t_frame);
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImageIndex=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<320>";
	pop_err();
	return dbg_object(this).m_imageIndex;
}
c_iLayerSprite.prototype.p_ImageIndex2=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<324>";
	dbg_object(this).m_imageIndex=t_index;
	pop_err();
}
c_iLayerSprite.prototype.p_ImageFrame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<400>";
	pop_err();
	return dbg_object(this).m_frame;
}
c_iLayerSprite.prototype.p_Frame2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<290>";
	if(dbg_object(this).m_imagePointer.length>1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<292>";
		var t_=((this.p_ImageIndex())|0);
		pop_err();
		return t_;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<296>";
		var t_2=((this.p_ImageFrame())|0);
		pop_err();
		return t_2;
	}
}
c_iLayerSprite.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<257>";
	c_iLayerObject.prototype.p_Destroy.call(this);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<259>";
	dbg_object(this).m_imagePointer=[];
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<260>";
	dbg_object(this).m_ghostImagePointer=[];
	pop_err();
}
c_iLayerSprite.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<395>";
	if(!((dbg_array(dbg_object(this).m_imagePointer,((dbg_object(this).m_imageIndex)|0))[dbg_index])!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<395>";
		pop_err();
		return 0.0;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<396>";
	var t_=bb_math_Abs2((dbg_array(dbg_object(this).m_imagePointer,((dbg_object(this).m_imageIndex)|0))[dbg_index].p_Height())*dbg_object(this).m_scaleY);
	pop_err();
	return t_;
}
c_iLayerSprite.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<758>";
	if(!((dbg_array(dbg_object(this).m_imagePointer,((dbg_object(this).m_imageIndex)|0))[dbg_index])!=null)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<758>";
		pop_err();
		return 0.0;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<759>";
	var t_=bb_math_Abs2((dbg_array(dbg_object(this).m_imagePointer,((dbg_object(this).m_imageIndex)|0))[dbg_index].p_Width())*dbg_object(this).m_scaleX);
	pop_err();
	return t_;
}
c_iLayerSprite.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<646>";
	if(dbg_object(this).m_ghost){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<649>";
		if(bb_graphics_GetBlend()!=dbg_object(this).m_ghostBlend){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<649>";
			bb_graphics_SetBlend(dbg_object(this).m_ghostBlend);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<653>";
		this.p_SetAlpha(dbg_object(this).m_ghostAlpha);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<657>";
		this.p_SetColor(dbg_object(this).m_ghostRed,dbg_object(this).m_ghostGreen,dbg_object(this).m_ghostBlue);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<660>";
		bb_graphics_DrawImage2(dbg_array(dbg_object(this).m_ghostImagePointer,((this.p_ImageIndex())|0))[dbg_index],this.p_RenderX(),this.p_RenderY(),this.p_Rotation()*(bb_globals_iRotation),this.p_ScaleX(),this.p_ScaleY(),((this.p_ImageFrame())|0));
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<662>";
		dbg_object(this).m_ghost=false;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<667>";
		if(bb_graphics_GetBlend()!=dbg_object(this).m_blend){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<667>";
			bb_graphics_SetBlend(dbg_object(this).m_blend);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<671>";
		this.p_SetAlpha(this.p_Alpha());
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<675>";
		this.p_SetColor(this.p_Red(),this.p_Green(),this.p_Blue());
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<678>";
		bb_graphics_DrawImage2(dbg_array(dbg_object(this).m_imagePointer,((this.p_ImageIndex())|0))[dbg_index],this.p_RenderX(),this.p_RenderY(),this.p_Rotation()*(bb_globals_iRotation),this.p_ScaleX(),this.p_ScaleY(),((this.p_ImageFrame())|0));
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<682>";
	if(((dbg_object(this).m_debugInfo).length!=0) && ((dbg_array(this.m_debugInfo,0)[dbg_index]).length!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<684>";
		if(bb_graphics_GetBlend()!=dbg_object(this).m_blend){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<684>";
			bb_graphics_SetBlend(dbg_object(this).m_blend);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<686>";
		this.p_SetAlpha(this.p_Alpha());
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<687>";
		this.p_SetColor(this.p_Red(),this.p_Green(),this.p_Blue());
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<689>";
		for(var t_i=0;t_i<this.m_debugInfo.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<690>";
			if((dbg_array(dbg_object(this).m_debugInfo,t_i)[dbg_index]).length!=0){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<691>";
				bb_graphics_DrawText(dbg_array(dbg_object(this).m_debugInfo,t_i)[dbg_index],this.p_RenderX()-this.p_Width()*.5,this.p_RenderY()+this.p_Height()*.5+(t_i*12),0.0,0.0);
			}
		}
	}
	pop_err();
}
c_iLayerSprite.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<746>";
	if(((dbg_object(this).m_debugInfo).length!=0) && ((dbg_array(dbg_object(this).m_debugInfo,0)[dbg_index]).length!=0)){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<747>";
		for(var t_i=0;t_i<dbg_object(this).m_debugInfo.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layersprite.monkey<748>";
			dbg_array(dbg_object(this).m_debugInfo,t_i)[dbg_index]="";
		}
	}
	pop_err();
}
function c_iLayerGroup(){
	c_iLayerObject.call(this);
	this.m_objectList=c_iList3.m_new.call(new c_iList3);
	this.implments={c_IMover:1};
}
c_iLayerGroup.prototype=extend_class(c_iLayerObject);
c_iLayerGroup.prototype.p_AddItemLast=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<32>";
	dbg_object(this).m_objectList.p_AddLast3(t_layerObject);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<34>";
	t_layerObject.p_Layer2(this.p_Layer());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<36>";
	t_layerObject.p_Parent2(this);
	pop_err();
}
c_iLayerGroup.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<104>";
	var t_=(dbg_object(this).m_height)*dbg_object(this).m_scaleY;
	pop_err();
	return t_;
}
c_iLayerGroup.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<150>";
	this.p_RenderList(this.m_objectList);
	pop_err();
}
c_iLayerGroup.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<218>";
	this.p_UpdateList(dbg_object(this).m_objectList);
	pop_err();
}
c_iLayerGroup.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<242>";
	var t_=(dbg_object(this).m_width)*dbg_object(this).m_scaleX;
	pop_err();
	return t_;
}
c_iLayerGroup.prototype.p_UpdateWorldXY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<226>";
	dbg_object(this).m_worldHeight=bb_math_Abs2(this.p_Height());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<227>";
	dbg_object(this).m_worldScaleX=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<228>";
	dbg_object(this).m_worldScaleY=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<229>";
	dbg_object(this).m_worldRotation=this.p_Rotation();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<230>";
	dbg_object(this).m_worldWidth=bb_math_Abs2(this.p_Width());
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<231>";
	dbg_object(this).m_worldX=dbg_object(this).m_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<232>";
	dbg_object(this).m_worldY=dbg_object(this).m_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/engine/components/layergroup.monkey<234>";
	this.p_UpdateWorldXYList(dbg_object(this).m_objectList);
	pop_err();
}
function c_iContentManager(){
	Object.call(this);
	this.m_data=new_object_array(128);
	this.m_length=0;
	this.m_cache=0;
}
c_iContentManager.prototype.p_SystemInit=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<521>";
	for(var t_i=0;t_i<dbg_object(this).m_data.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<522>";
		dbg_array(dbg_object(this).m_data,t_i)[dbg_index]=c_iContentObject.m_new.call(new c_iContentObject);
	}
	pop_err();
}
c_iContentManager.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<472>";
	this.p_SystemInit();
	pop_err();
	return this;
}
c_iContentManager.prototype.p_Expand=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<376>";
	if(dbg_object(this).m_length==dbg_object(this).m_data.length){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<378>";
		dbg_object(this).m_data=resize_object_array(dbg_object(this).m_data,dbg_object(this).m_length*2+10);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<380>";
		for(var t_i=0;t_i<dbg_object(this).m_data.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<381>";
			if(!((dbg_array(dbg_object(this).m_data,t_i)[dbg_index])!=null)){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<381>";
				dbg_array(dbg_object(this).m_data,t_i)[dbg_index]=c_iContentObject.m_new.call(new c_iContentObject);
			}
		}
	}
	pop_err();
}
c_iContentManager.prototype.p_Get2=function(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<70>";
	for(this.m_cache=0;this.m_cache<dbg_object(this).m_length;this.m_cache=this.m_cache+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<71>";
		if(dbg_object(dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index]).m_path==t_path){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<71>";
			pop_err();
			return dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index];
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<73>";
	pop_err();
	return null;
}
c_iContentManager.prototype.p_GetImage=function(t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<160>";
	if(dbg_object(this).m_length>0 && ((dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index])!=null) && dbg_object(dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index]).m_path==t_path){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<160>";
		pop_err();
		return dbg_object(dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index]).m_imagePointer;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<162>";
	this.p_Expand();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<164>";
	var t_o=this.p_Get2(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<166>";
	if((t_o)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<168>";
		pop_err();
		return dbg_object(t_o).m_imagePointer;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<172>";
		t_o=dbg_array(dbg_object(this).m_data,dbg_object(this).m_length)[dbg_index];
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<174>";
		dbg_object(this).m_length=dbg_object(this).m_length+1;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<176>";
		dbg_object(t_o).m_imagePointer=bb_graphics_LoadImage(t_path,t_frameCount,t_flags);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<178>";
		dbg_object(t_o).m_path=t_path;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<180>";
		pop_err();
		return dbg_object(t_o).m_imagePointer;
	}
}
c_iContentManager.prototype.p_GetImage2=function(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<205>";
	if(dbg_object(this).m_length>0 && ((dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index])!=null) && dbg_object(dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index]).m_path==t_path){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<205>";
		pop_err();
		return dbg_object(dbg_array(this.m_data,dbg_object(this).m_cache)[dbg_index]).m_imagePointer;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<207>";
	this.p_Expand();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<209>";
	var t_o=this.p_Get2(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<211>";
	if((t_o)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<213>";
		pop_err();
		return dbg_object(t_o).m_imagePointer;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<217>";
		t_o=dbg_array(this.m_data,dbg_object(this).m_length)[dbg_index];
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<219>";
		dbg_object(this).m_length=dbg_object(this).m_length+1;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<221>";
		dbg_object(t_o).m_imagePointer=bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<223>";
		dbg_object(t_o).m_path=t_path;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<225>";
		pop_err();
		return dbg_object(t_o).m_imagePointer;
	}
}
c_iContentManager.prototype.p_GetImage3=function(t_start,t_count,t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<242>";
	if(dbg_object(this).m_length>0 && ((dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index])!=null) && dbg_object(dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index]).m_path==t_path){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<242>";
		pop_err();
		return dbg_object(dbg_array(this.m_data,dbg_object(this).m_cache)[dbg_index]).m_imagePointers;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<244>";
	this.p_Expand();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<246>";
	var t_o=this.p_Get2(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<248>";
	if((t_o)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<250>";
		pop_err();
		return dbg_object(t_o).m_imagePointers;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<254>";
		t_o=dbg_array(this.m_data,dbg_object(this).m_length)[dbg_index];
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<256>";
		dbg_object(this).m_length=dbg_object(this).m_length+1;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<258>";
		dbg_object(t_o).m_imagePointers=bb_gfx_iLoadImage(t_start,t_count,t_path,t_frameCount,t_flags);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<260>";
		dbg_object(t_o).m_path=t_path;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<262>";
		pop_err();
		return dbg_object(t_o).m_imagePointers;
	}
}
c_iContentManager.prototype.p_GetImage4=function(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<279>";
	if(dbg_object(this).m_length>0 && ((dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index])!=null) && dbg_object(dbg_array(dbg_object(this).m_data,dbg_object(this).m_cache)[dbg_index]).m_path==t_path){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<279>";
		pop_err();
		return dbg_object(dbg_array(this.m_data,dbg_object(this).m_cache)[dbg_index]).m_imagePointers;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<281>";
	this.p_Expand();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<283>";
	var t_o=this.p_Get2(t_path);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<285>";
	if((t_o)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<287>";
		pop_err();
		return dbg_object(t_o).m_imagePointers;
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<291>";
		t_o=dbg_array(this.m_data,dbg_object(this).m_length)[dbg_index];
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<293>";
		dbg_object(this).m_length=dbg_object(this).m_length+1;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<295>";
		dbg_object(t_o).m_imagePointers=bb_gfx_iLoadImage3(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<297>";
		dbg_object(t_o).m_path=t_path;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<299>";
		pop_err();
		return dbg_object(t_o).m_imagePointers;
	}
}
function c_iContentObject(){
	Object.call(this);
	this.m_path="";
	this.m_imagePointer=null;
	this.m_imagePointers=[];
}
c_iContentObject.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/playniax/ignitionx/framework/contentmanager.monkey<542>";
	pop_err();
	return this;
}
var bb_contentmanager_iContent=null;
function c_Player(){
	Object.call(this);
	this.m_image=null;
	this.m_x=0;
	this.m_y=0;
	this.m_xVel=0;
	this.m_yVel=0;
	this.m_downAnim=null;
	this.m_currentAnimation=null;
	this.m_frameNum=0;
}
c_Player.m_new=function(t_i,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<15>";
	dbg_object(this).m_image=t_i;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<16>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<17>";
	dbg_object(this).m_y=t_y;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<18>";
	dbg_object(this).m_xVel=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<19>";
	dbg_object(this).m_yVel=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<20>";
	dbg_object(this).m_downAnim=c_Animation.m_new.call(new c_Animation,t_i,4,69,102,200);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<21>";
	dbg_object(this).m_currentAnimation=dbg_object(this).m_downAnim;
	pop_err();
	return this;
}
c_Player.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<4>";
	pop_err();
	return this;
}
c_Player.prototype.p_HandleControls=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<34>";
	if((bb_input_KeyDown(37))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<35>";
		this.m_x=this.m_x-4;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<37>";
	if((bb_input_KeyDown(39))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<38>";
		this.m_x=this.m_x+4;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<40>";
	if((bb_input_KeyDown(40))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<41>";
		this.m_y=this.m_y+4;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<43>";
	if((bb_input_KeyDown(38))!=0){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<44>";
		this.m_y=this.m_y-4;
	}
	pop_err();
	return 0;
}
c_Player.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<25>";
	this.p_HandleControls();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/player.monkey<26>";
	this.m_frameNum=this.m_currentAnimation.p_getFrame();
	pop_err();
	return 0;
}
function c_Animation(){
	Object.call(this);
	this.m_img=null;
	this.m_frames=0;
	this.m_width=0;
	this.m_height=0;
	this.m_frameTime=0;
	this.m_elapsed=0;
	this.m_frame=0;
	this.m_lastTime=0;
}
c_Animation.m_new=function(t_i,t_f,t_w,t_h,t_ft){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<15>";
	dbg_object(this).m_img=t_i;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<16>";
	dbg_object(this).m_frames=t_f;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<17>";
	dbg_object(this).m_width=t_w;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<18>";
	dbg_object(this).m_height=t_h;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<19>";
	dbg_object(this).m_frameTime=t_ft;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<21>";
	dbg_object(this).m_elapsed=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<22>";
	dbg_object(this).m_frame=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<23>";
	dbg_object(this).m_lastTime=0;
	pop_err();
	return this;
}
c_Animation.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<4>";
	pop_err();
	return this;
}
c_Animation.prototype.p_getFrame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<27>";
	this.m_elapsed=this.m_elapsed+(bb_app_Millisecs()-this.m_lastTime);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<28>";
	this.m_lastTime=bb_app_Millisecs();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<30>";
	var t_toReturn=((Math.floor((this.m_elapsed/this.m_frameTime)|0))|0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<32>";
	if(t_toReturn>this.m_frames-1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<33>";
		t_toReturn=0;
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<34>";
		this.m_elapsed=0;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/animation.monkey<37>";
	pop_err();
	return t_toReturn;
}
function c_Sound(){
	Object.call(this);
	this.m_sample=null;
}
c_Sound.m_new=function(t_sample){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/audio.monkey<32>";
	dbg_object(this).m_sample=t_sample;
	pop_err();
	return this;
}
c_Sound.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/audio.monkey<29>";
	pop_err();
	return this;
}
function bb_audio_LoadSound(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/audio.monkey<47>";
	var t_sample=bb_audio_device.LoadSample(bb_data_FixDataPath(t_path));
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/audio.monkey<48>";
	if((t_sample)!=null){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/audio.monkey<48>";
		var t_=c_Sound.m_new.call(new c_Sound,t_sample);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/audio.monkey<49>";
	pop_err();
	return null;
}
function c_Level(){
	Object.call(this);
	this.m_layout=[];
	this.m_generated=false;
	this.m_width=0;
	this.m_height=0;
}
c_Level.prototype.p_Draw=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<207>";
	for(var t_i=0;t_i<dbg_object(this).m_layout.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<208>";
		for(var t_j=0;t_j<dbg_array(dbg_object(this).m_layout,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<209>";
			if(dbg_array(dbg_array(dbg_object(this).m_layout,t_i)[dbg_index],t_j)[dbg_index]=="X"){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<210>";
				bb_graphics_SetColor(0.0,0.0,0.0);
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<212>";
				bb_graphics_SetColor(255.0,255.0,255.0);
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<214>";
			bb_graphics_DrawRect((t_i*10),(t_j*10),10.0,10.0);
		}
	}
	pop_err();
	return 0;
}
c_Level.prototype.p_setArray=function(t_i,t_j){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<44>";
	var t_result=new_array_array(t_i);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<46>";
	for(var t_index=0;t_index<t_i;t_index=t_index+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<47>";
		dbg_array(t_result,t_index)[dbg_index]=new_string_array(t_j);
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<50>";
	pop_err();
	return t_result;
}
c_Level.prototype.p_randomlyAssignCells=function(t_design){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<57>";
	bb_random_Seed=bb_app_Millisecs();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<58>";
	var t_rand=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<60>";
	for(var t_i=0;t_i<t_design.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<61>";
		for(var t_j=0;t_j<dbg_array(t_design,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<62>";
			t_rand=((bb_random_Rnd2(0.0,100.0))|0);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<63>";
			if(t_rand<45){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<64>";
				dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]="X";
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<66>";
				dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]="O";
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<68>";
			if(t_i==0 || t_j==0 || t_i==t_design.length-1 || t_j==dbg_array(t_design,0)[dbg_index].length-1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<69>";
				dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]="X";
			}
		}
	}
	pop_err();
	return 0;
}
c_Level.prototype.p_checkWalls=function(t_design,t_i,t_j){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<114>";
	var t_total=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<115>";
	if(t_i>0 && dbg_array(dbg_array(t_design,t_i-1)[dbg_index],t_j)[dbg_index]=="X"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<116>";
		t_total+=1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<118>";
	if(t_i<t_design.length-1 && dbg_array(dbg_array(t_design,t_i+1)[dbg_index],t_j)[dbg_index]=="X"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<119>";
		t_total+=1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<121>";
	if(t_j>0 && dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j-1)[dbg_index]=="X"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<122>";
		t_total+=1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<124>";
	if(t_j<dbg_array(t_design,0)[dbg_index].length-1 && dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j+1)[dbg_index]=="X"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<125>";
		t_total+=1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<127>";
	if(t_i>0 && t_j<dbg_array(t_design,0)[dbg_index].length-1 && dbg_array(dbg_array(t_design,t_i-1)[dbg_index],t_j+1)[dbg_index]=="X"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<128>";
		t_total+=1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<130>";
	if(t_i>0 && t_j>0 && dbg_array(dbg_array(t_design,t_i-1)[dbg_index],t_j-1)[dbg_index]=="X"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<131>";
		t_total+=1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<133>";
	if(t_i<t_design.length-1 && t_j<dbg_array(t_design,0)[dbg_index].length-1 && dbg_array(dbg_array(t_design,t_i+1)[dbg_index],t_j+1)[dbg_index]=="X"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<134>";
		t_total+=1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<136>";
	if(t_i<t_design.length-1 && t_j>0 && dbg_array(dbg_array(t_design,t_i+1)[dbg_index],t_j-1)[dbg_index]=="X"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<137>";
		t_total+=1;
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<140>";
	pop_err();
	return t_total;
}
c_Level.prototype.p_generateCellularly=function(t_design){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<81>";
	var t_adjacentWalls=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<82>";
	var t_result=this.p_setArray(t_design.length,dbg_array(t_design,0)[dbg_index].length);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<84>";
	for(var t_i=0;t_i<t_design.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<85>";
		for(var t_j=0;t_j<dbg_array(t_design,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<86>";
			t_adjacentWalls=this.p_checkWalls(t_design,t_i,t_j);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<88>";
			if(dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]=="X"){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<89>";
				if(t_adjacentWalls>3){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<90>";
					dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="X";
				}else{
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<92>";
					dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="O";
				}
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<94>";
				if(dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]=="O"){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<95>";
					if(t_adjacentWalls>4){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<96>";
						dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="X";
					}else{
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<98>";
						dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="O";
					}
				}
			}
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<102>";
			if(t_i==0 || t_j==0 || t_i==t_design.length-1 || t_j==dbg_array(t_design,0)[dbg_index].length-1){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<103>";
				dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="X";
			}
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<108>";
	pop_err();
	return t_result;
}
c_Level.prototype.p_fillCells=function(t_design){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<161>";
	for(var t_i=0;t_i<t_design.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<162>";
		for(var t_j=0;t_j<dbg_array(t_design,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<163>";
			dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]="X";
		}
	}
	pop_err();
	return 0;
}
c_Level.prototype.p_drunkWalk=function(t_design){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<170>";
	var t_target=(t_design.length*dbg_array(t_design,0)[dbg_index].length)*0.35;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<171>";
	var t_cleared=1;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<173>";
	var t_tempX=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<174>";
	var t_tempY=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<175>";
	var t_direction=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<177>";
	var t_followBias=0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<178>";
	t_tempX=((bb_random_Rnd2(1.0,(t_design.length-1)))|0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<179>";
	t_tempY=((bb_random_Rnd2(1.0,(dbg_array(t_design,0)[dbg_index].length-1)))|0);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<181>";
	dbg_array(dbg_array(t_design,t_tempX)[dbg_index],t_tempY)[dbg_index]="O";
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<183>";
	while((t_cleared)<t_target){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<184>";
		if(t_followBias<40){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<185>";
			t_direction=((bb_random_Rnd2(0.0,4.0))|0);
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<187>";
		if(t_direction==0 && t_tempY>1){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<188>";
			t_tempY-=1;
		}else{
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<189>";
			if(t_direction==1 && t_tempY<dbg_array(t_design,0)[dbg_index].length-2){
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<190>";
				t_tempY+=1;
			}else{
				err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<191>";
				if(t_direction==2 && t_tempX>1){
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<192>";
					t_tempX-=1;
				}else{
					err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<193>";
					if(t_direction==3 && t_tempX<t_design.length-2){
						err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<194>";
						t_tempX+=1;
					}
				}
			}
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<197>";
		if(dbg_array(dbg_array(t_design,t_tempX)[dbg_index],t_tempY)[dbg_index]=="X"){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<198>";
			dbg_array(dbg_array(t_design,t_tempX)[dbg_index],t_tempY)[dbg_index]="O";
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<199>";
			t_cleared+=1;
		}
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<201>";
		t_followBias=((bb_random_Rnd2(0.0,100.0))|0);
	}
	pop_err();
	return 0;
}
c_Level.m_new=function(t_w,t_h,t_type){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<12>";
	dbg_object(this).m_generated=false;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<13>";
	dbg_object(this).m_width=t_w;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<14>";
	dbg_object(this).m_height=t_h;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<15>";
	dbg_object(this).m_layout=this.p_setArray(dbg_object(this).m_width,dbg_object(this).m_height);
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<17>";
	if(t_type=="Cellular"){
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<18>";
		this.p_randomlyAssignCells(dbg_object(this).m_layout);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<19>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<20>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<21>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<22>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<23>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
	}else{
		err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<24>";
		if(t_type=="Drunk"){
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<28>";
			this.p_fillCells(dbg_object(this).m_layout);
			err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<29>";
			this.p_drunkWalk(dbg_object(this).m_layout);
		}
	}
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<31>";
	dbg_object(this).m_generated=true;
	pop_err();
	return this;
}
c_Level.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/MonkeyTest/level.monkey<4>";
	pop_err();
	return this;
}
function bb_graphics_DrawRect(t_x,t_y,t_w,t_h){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<393>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<395>";
	bb_graphics_context.p_Validate();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/graphics.monkey<396>";
	bb_graphics_renderDevice.DrawRect(t_x,t_y,t_w,t_h);
	pop_err();
	return 0;
}
function bb_random_Rnd(){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/random.monkey<21>";
	bb_random_Seed=bb_random_Seed*1664525+1013904223|0;
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/random.monkey<22>";
	var t_=(bb_random_Seed>>8&16777215)/16777216.0;
	pop_err();
	return t_;
}
function bb_random_Rnd2(t_low,t_high){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/random.monkey<30>";
	var t_=bb_random_Rnd3(t_high-t_low)+t_low;
	pop_err();
	return t_;
}
function bb_random_Rnd3(t_range){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/monkey/random.monkey<26>";
	var t_=bb_random_Rnd()*t_range;
	pop_err();
	return t_;
}
function bb_input_KeyDown(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro80c+IgnitionX2.04/modules/mojo/input.monkey<40>";
	var t_=((bb_input_device.p_KeyDown(t_key))?1:0);
	pop_err();
	return t_;
}
function bbInit(){
	bb_app__app=null;
	bb_app__delegate=null;
	bb_app__game=BBGame.Game();
	bb_graphics_device=null;
	bb_graphics_context=c_GraphicsContext.m_new.call(new c_GraphicsContext);
	c_Image.m_DefaultFlags=0;
	bb_audio_device=null;
	bb_input_device=null;
	bb_app__devWidth=0;
	bb_app__devHeight=0;
	bb_app__displayModes=[];
	bb_app__desktopMode=null;
	bb_graphics_renderDevice=null;
	bb_app2_iCurrentScene=null;
	bb_app2_iDT=c_iDeltaTimer.m_new.call(new c_iDeltaTimer,60.0);
	bb_app2_iNextScene=null;
	bb_random_Seed=1234;
	bb_main_gfxMonkey=null;
	bb_main_menu=null;
	bb_main_gameplay=null;
	bb_app__updateRate=0;
	bb_globals_iEnginePointer=null;
	c_iStack2D.m_length=0;
	c_iStack2D.m_data=[];
	bb_globals_iRotation=-1;
	c_iSystemGui.m_playfield=null;
	bb_globals_iTaskList=c_iList5.m_new.call(new c_iList5);
	c_iGuiObject.m_topObject=null;
	bb_globals_iMultiTouch=8;
	bb_contentmanager_iContent=c_iContentManager.m_new.call(new c_iContentManager);
}
//${TRANSCODE_END}
