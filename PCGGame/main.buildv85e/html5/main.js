
//${CONFIG_BEGIN}
CFG_BINARY_FILES="*.bin|*.dat";
CFG_BRL_GAMETARGET_IMPLEMENTED="1";
CFG_BRL_THREAD_IMPLEMENTED="1";
CFG_CD="";
CFG_CONFIG="debug";
CFG_HOST="macos";
CFG_HTML5_WEBAUDIO_ENABLED="1";
CFG_IGNITION_ALLOW_SET_ALPHA_FOR_EACH_OBJECT="1";
CFG_IGNITION_ALLOW_SET_BLEND_FOR_EACH_OBJECT="1";
CFG_IGNITION_ALLOW_SET_COLOR_FOR_EACH_OBJECT="1";
CFG_IGNITION_DEBUG_TOOLS="0";
CFG_IMAGE_FILES="*.png|*.jpg";
CFG_LANG="js";
CFG_MODPATH="";
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
var META_DATA="[mojo_font.png];type=image/png;width=864;height=13;\n[char_walk_down.png];type=image/png;width=276;height=102;\n[chocobo.png];type=image/png;width=60;height=64;\n[enemies20.png];type=image/png;width=200;height=20;\n[rockstar.png];type=image/png;width=69;height=102;\n[textures.png];type=image/png;width=100;height=10;\n[textures20.png];type=image/png;width=200;height=20;\n";
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


var webglGraphicsSeq=1;

function BBHtml5Game( canvas ){

	BBGame.call( this );
	BBHtml5Game._game=this;
	this._canvas=canvas;
	this._loading=0;
	this._timerSeq=0;
	this._gl=null;
	
	if( CFG_OPENGL_GLES20_ENABLED=="1" ){

		//can't get these to fire!
		canvas.addEventListener( "webglcontextlost",function( event ){
			event.preventDefault();
//			print( "WebGL context lost!" );
		},false );

		canvas.addEventListener( "webglcontextrestored",function( event ){
			++webglGraphicsSeq;
//			print( "WebGL context restored!" );
		},false );

		var attrs={ alpha:false };
	
		this._gl=this._canvas.getContext( "webgl",attrs );

		if( !this._gl ) this._gl=this._canvas.getContext( "experimental-webgl",attrs );
		
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
	this.waPanner.panningModel="equalpower";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<152>";
	if((bb_app__app)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<152>";
		error("App has already been created");
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<153>";
	bb_app__app=this;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<154>";
	bb_app__delegate=c_GameDelegate.m_new.call(new c_GameDelegate);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<155>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<177>";
	bb_app_EndApp();
	pop_err();
	return 0;
}
c_App.prototype.p_OnBack=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<181>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<294>";
	c_App.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<294>";
	pop_err();
	return this;
}
c_iApp.prototype.p_OnBack=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<297>";
	bb_app2_iBack();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<298>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnClose=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<302>";
	bb_app2_iClose();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<303>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnLoading=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<307>";
	bb_app2_iLoading();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<308>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<312>";
	bb_app2_iRender();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<313>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnResize=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<317>";
	bb_app2_iResize();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<318>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnResume=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<322>";
	bb_app2_iResume();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<323>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnSuspend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<327>";
	bb_app2_iSuspend();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<328>";
	pop_err();
	return 0;
}
c_iApp.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<332>";
	bb_app2_iUpdate();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<333>";
	pop_err();
	return 0;
}
function c_Game(){
	c_iApp.call(this);
}
c_Game.prototype=extend_class(c_iApp);
c_Game.m_new=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<20>";
	c_iApp.m_new.call(this);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<20>";
	pop_err();
	return this;
}
c_Game.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<25>";
	bb_random_Seed=bb_app_Millisecs();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<33>";
	bb_main_menu=c_MenuScene.m_new.call(new c_MenuScene);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<34>";
	bb_main_gameplay=c_GameplayScene.m_new.call(new c_GameplayScene);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<35>";
	bb_main_noiseTest=c_NoiseTestScene.m_new.call(new c_NoiseTestScene);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<38>";
	bb_app2_iStart3((bb_main_menu),60);
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<65>";
	pop_err();
	return this;
}
c_GameDelegate.prototype.StartGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<75>";
	this.m__graphics=(new gxtkGraphics);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<76>";
	bb_graphics_SetGraphicsDevice(this.m__graphics);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<77>";
	bb_graphics_SetFont(null,32);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<79>";
	this.m__audio=(new gxtkAudio);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<80>";
	bb_audio_SetAudioDevice(this.m__audio);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<82>";
	this.m__input=c_InputDevice.m_new.call(new c_InputDevice);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<83>";
	bb_input_SetInputDevice(this.m__input);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<85>";
	bb_app_ValidateDeviceWindow(false);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<87>";
	bb_app_EnumDisplayModes();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<89>";
	bb_app__app.p_OnCreate();
	pop_err();
}
c_GameDelegate.prototype.SuspendGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<93>";
	bb_app__app.p_OnSuspend();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<94>";
	this.m__audio.Suspend();
	pop_err();
}
c_GameDelegate.prototype.ResumeGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<98>";
	this.m__audio.Resume();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<99>";
	bb_app__app.p_OnResume();
	pop_err();
}
c_GameDelegate.prototype.UpdateGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<103>";
	bb_app_ValidateDeviceWindow(true);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<104>";
	this.m__input.p_BeginUpdate();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<105>";
	bb_app__app.p_OnUpdate();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<106>";
	this.m__input.p_EndUpdate();
	pop_err();
}
c_GameDelegate.prototype.RenderGame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<110>";
	bb_app_ValidateDeviceWindow(true);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<111>";
	var t_mode=this.m__graphics.BeginRender();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<112>";
	if((t_mode)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<112>";
		bb_graphics_BeginRender();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<113>";
	if(t_mode==2){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<113>";
		bb_app__app.p_OnLoading();
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<113>";
		bb_app__app.p_OnRender();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<114>";
	if((t_mode)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<114>";
		bb_graphics_EndRender();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<115>";
	this.m__graphics.EndRender();
	pop_err();
}
c_GameDelegate.prototype.KeyEvent=function(t_event,t_data){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<119>";
	this.m__input.p_KeyEvent(t_event,t_data);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<120>";
	if(t_event!=1){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<121>";
	var t_1=t_data;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<122>";
	if(t_1==432){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<123>";
		bb_app__app.p_OnClose();
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<124>";
		if(t_1==416){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<125>";
			bb_app__app.p_OnBack();
		}
	}
	pop_err();
}
c_GameDelegate.prototype.MouseEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<130>";
	this.m__input.p_MouseEvent(t_event,t_data,t_x,t_y);
	pop_err();
}
c_GameDelegate.prototype.TouchEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<134>";
	this.m__input.p_TouchEvent(t_event,t_data,t_x,t_y);
	pop_err();
}
c_GameDelegate.prototype.MotionEvent=function(t_event,t_data,t_x,t_y,t_z){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<138>";
	this.m__input.p_MotionEvent(t_event,t_data,t_x,t_y,t_z);
	pop_err();
}
c_GameDelegate.prototype.DiscardGraphics=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<142>";
	this.m__graphics.DiscardGraphics();
	pop_err();
}
var bb_app__delegate=null;
var bb_app__game=null;
function bbMain(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<238>";
	var t_g=c_Game.m_new.call(new c_Game);
	pop_err();
	return 0;
}
var bb_graphics_device=null;
function bb_graphics_SetGraphicsDevice(t_dev){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<63>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<70>";
	pop_err();
	return this;
}
c_Image.prototype.p_SetHandle=function(t_tx,t_ty){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<114>";
	dbg_object(this).m_tx=t_tx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<115>";
	dbg_object(this).m_ty=t_ty;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<116>";
	dbg_object(this).m_flags=dbg_object(this).m_flags&-2;
	pop_err();
	return 0;
}
c_Image.prototype.p_ApplyFlags=function(t_iflags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<197>";
	this.m_flags=t_iflags;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<199>";
	if((this.m_flags&2)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<200>";
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<200>";
		var t_=this.m_frames;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<200>";
		var t_2=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<200>";
		while(t_2<t_.length){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<200>";
			var t_f=dbg_array(t_,t_2)[dbg_index];
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<200>";
			t_2=t_2+1;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<201>";
			dbg_object(t_f).m_x+=1;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<203>";
		this.m_width-=2;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<206>";
	if((this.m_flags&4)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<207>";
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<207>";
		var t_3=this.m_frames;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<207>";
		var t_4=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<207>";
		while(t_4<t_3.length){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<207>";
			var t_f2=dbg_array(t_3,t_4)[dbg_index];
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<207>";
			t_4=t_4+1;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<208>";
			dbg_object(t_f2).m_y+=1;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<210>";
		this.m_height-=2;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<213>";
	if((this.m_flags&1)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<214>";
		this.p_SetHandle((this.m_width)/2.0,(this.m_height)/2.0);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<217>";
	if(this.m_frames.length==1 && dbg_object(dbg_array(this.m_frames,0)[dbg_index]).m_x==0 && dbg_object(dbg_array(this.m_frames,0)[dbg_index]).m_y==0 && this.m_width==this.m_surface.Width() && this.m_height==this.m_surface.Height()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<218>";
		this.m_flags|=65536;
	}
	pop_err();
	return 0;
}
c_Image.prototype.p_Init=function(t_surf,t_nframes,t_iflags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<143>";
	if((this.m_surface)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<143>";
		error("Image already initialized");
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<144>";
	this.m_surface=t_surf;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<146>";
	this.m_width=((this.m_surface.Width()/t_nframes)|0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<147>";
	this.m_height=this.m_surface.Height();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<149>";
	this.m_frames=new_object_array(t_nframes);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<150>";
	for(var t_i=0;t_i<t_nframes;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<151>";
		dbg_array(this.m_frames,t_i)[dbg_index]=c_Frame.m_new.call(new c_Frame,t_i*this.m_width,0);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<154>";
	this.p_ApplyFlags(t_iflags);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<155>";
	pop_err();
	return this;
}
c_Image.prototype.p_Init2=function(t_surf,t_x,t_y,t_iwidth,t_iheight,t_nframes,t_iflags,t_src,t_srcx,t_srcy,t_srcw,t_srch){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<159>";
	if((this.m_surface)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<159>";
		error("Image already initialized");
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<160>";
	this.m_surface=t_surf;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<161>";
	this.m_source=t_src;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<163>";
	this.m_width=t_iwidth;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<164>";
	this.m_height=t_iheight;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<166>";
	this.m_frames=new_object_array(t_nframes);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<168>";
	var t_ix=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<168>";
	var t_iy=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<170>";
	for(var t_i=0;t_i<t_nframes;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<171>";
		if(t_ix+this.m_width>t_srcw){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<172>";
			t_ix=0;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<173>";
			t_iy+=this.m_height;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<175>";
		if(t_ix+this.m_width>t_srcw || t_iy+this.m_height>t_srch){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<176>";
			error("Image frame outside surface");
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<178>";
		dbg_array(this.m_frames,t_i)[dbg_index]=c_Frame.m_new.call(new c_Frame,t_ix+t_srcx,t_iy+t_srcy);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<179>";
		t_ix+=this.m_width;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<182>";
	this.p_ApplyFlags(t_iflags);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<183>";
	pop_err();
	return this;
}
c_Image.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<81>";
	pop_err();
	return this.m_width;
}
c_Image.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<85>";
	pop_err();
	return this.m_height;
}
c_Image.prototype.p_Frames=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<93>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<29>";
	pop_err();
	return this;
}
c_GraphicsContext.prototype.p_Validate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<40>";
	if((this.m_matDirty)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<41>";
		bb_graphics_renderDevice.SetMatrix(dbg_object(bb_graphics_context).m_ix,dbg_object(bb_graphics_context).m_iy,dbg_object(bb_graphics_context).m_jx,dbg_object(bb_graphics_context).m_jy,dbg_object(bb_graphics_context).m_tx,dbg_object(bb_graphics_context).m_ty);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<42>";
		this.m_matDirty=0;
	}
	pop_err();
	return 0;
}
var bb_graphics_context=null;
function bb_data_FixDataPath(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<7>";
	var t_i=t_path.indexOf(":/",0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<8>";
	if(t_i!=-1 && t_path.indexOf("/",0)==t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<8>";
		pop_err();
		return t_path;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<9>";
	if(string_startswith(t_path,"./") || string_startswith(t_path,"/")){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<9>";
		pop_err();
		return t_path;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/data.monkey<10>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<23>";
	dbg_object(this).m_x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<24>";
	dbg_object(this).m_y=t_y;
	pop_err();
	return this;
}
c_Frame.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<18>";
	pop_err();
	return this;
}
function bb_graphics_LoadImage(t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<239>";
	var t_surf=bb_graphics_device.LoadSurface(bb_data_FixDataPath(t_path));
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<240>";
	if((t_surf)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<240>";
		var t_=(c_Image.m_new.call(new c_Image)).p_Init(t_surf,t_frameCount,t_flags);
		pop_err();
		return t_;
	}
	pop_err();
	return null;
}
function bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<244>";
	var t_surf=bb_graphics_device.LoadSurface(bb_data_FixDataPath(t_path));
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<245>";
	if((t_surf)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<245>";
		var t_=(c_Image.m_new.call(new c_Image)).p_Init2(t_surf,0,0,t_frameWidth,t_frameHeight,t_frameCount,t_flags,null,0,0,t_surf.Width(),t_surf.Height());
		pop_err();
		return t_;
	}
	pop_err();
	return null;
}
function bb_graphics_SetFont(t_font,t_firstChar){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<548>";
	if(!((t_font)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<549>";
		if(!((dbg_object(bb_graphics_context).m_defaultFont)!=null)){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<550>";
			dbg_object(bb_graphics_context).m_defaultFont=bb_graphics_LoadImage("mojo_font.png",96,2);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<552>";
		t_font=dbg_object(bb_graphics_context).m_defaultFont;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<553>";
		t_firstChar=32;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<555>";
	dbg_object(bb_graphics_context).m_font=t_font;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<556>";
	dbg_object(bb_graphics_context).m_firstChar=t_firstChar;
	pop_err();
	return 0;
}
var bb_audio_device=null;
function bb_audio_SetAudioDevice(t_dev){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<22>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<26>";
	for(var t_i=0;t_i<4;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<27>";
		dbg_array(this.m__joyStates,t_i)[dbg_index]=c_JoyState.m_new.call(new c_JoyState);
	}
	pop_err();
	return this;
}
c_InputDevice.prototype.p_PutKeyHit=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<237>";
	if(this.m__keyHitPut==this.m__keyHitQueue.length){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<238>";
	dbg_array(this.m__keyHit,t_key)[dbg_index]+=1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<239>";
	dbg_array(this.m__keyHitQueue,this.m__keyHitPut)[dbg_index]=t_key;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<240>";
	this.m__keyHitPut+=1;
	pop_err();
}
c_InputDevice.prototype.p_BeginUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<189>";
	for(var t_i=0;t_i<4;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<190>";
		var t_state=dbg_array(this.m__joyStates,t_i)[dbg_index];
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<191>";
		if(!BBGame.Game().PollJoystick(t_i,dbg_object(t_state).m_joyx,dbg_object(t_state).m_joyy,dbg_object(t_state).m_joyz,dbg_object(t_state).m_buttons)){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<191>";
			break;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<192>";
		for(var t_j=0;t_j<32;t_j=t_j+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<193>";
			var t_key=256+t_i*32+t_j;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<194>";
			if(dbg_array(dbg_object(t_state).m_buttons,t_j)[dbg_index]){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<195>";
				if(!dbg_array(this.m__keyDown,t_key)[dbg_index]){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<196>";
					dbg_array(this.m__keyDown,t_key)[dbg_index]=true;
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<197>";
					this.p_PutKeyHit(t_key);
				}
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<200>";
				dbg_array(this.m__keyDown,t_key)[dbg_index]=false;
			}
		}
	}
	pop_err();
}
c_InputDevice.prototype.p_EndUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<207>";
	for(var t_i=0;t_i<this.m__keyHitPut;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<208>";
		dbg_array(this.m__keyHit,dbg_array(this.m__keyHitQueue,t_i)[dbg_index])[dbg_index]=0;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<210>";
	this.m__keyHitPut=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<211>";
	this.m__charGet=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<212>";
	this.m__charPut=0;
	pop_err();
}
c_InputDevice.prototype.p_KeyEvent=function(t_event,t_data){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<111>";
	var t_1=t_event;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<112>";
	if(t_1==1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<113>";
		if(!dbg_array(this.m__keyDown,t_data)[dbg_index]){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<114>";
			dbg_array(this.m__keyDown,t_data)[dbg_index]=true;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<115>";
			this.p_PutKeyHit(t_data);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<116>";
			if(t_data==1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<117>";
				dbg_array(this.m__keyDown,384)[dbg_index]=true;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<118>";
				this.p_PutKeyHit(384);
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<119>";
				if(t_data==384){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<120>";
					dbg_array(this.m__keyDown,1)[dbg_index]=true;
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<121>";
					this.p_PutKeyHit(1);
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<124>";
		if(t_1==2){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<125>";
			if(dbg_array(this.m__keyDown,t_data)[dbg_index]){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<126>";
				dbg_array(this.m__keyDown,t_data)[dbg_index]=false;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<127>";
				if(t_data==1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<128>";
					dbg_array(this.m__keyDown,384)[dbg_index]=false;
				}else{
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<129>";
					if(t_data==384){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<130>";
						dbg_array(this.m__keyDown,1)[dbg_index]=false;
					}
				}
			}
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<133>";
			if(t_1==3){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<134>";
				if(this.m__charPut<this.m__charQueue.length){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<135>";
					dbg_array(this.m__charQueue,this.m__charPut)[dbg_index]=t_data;
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<136>";
					this.m__charPut+=1;
				}
			}
		}
	}
	pop_err();
}
c_InputDevice.prototype.p_MouseEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<142>";
	var t_2=t_event;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<143>";
	if(t_2==4){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<144>";
		this.p_KeyEvent(1,1+t_data);
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<145>";
		if(t_2==5){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<146>";
			this.p_KeyEvent(2,1+t_data);
			pop_err();
			return;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<148>";
			if(t_2==6){
			}else{
				pop_err();
				return;
			}
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<152>";
	this.m__mouseX=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<153>";
	this.m__mouseY=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<154>";
	dbg_array(this.m__touchX,0)[dbg_index]=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<155>";
	dbg_array(this.m__touchY,0)[dbg_index]=t_y;
	pop_err();
}
c_InputDevice.prototype.p_TouchEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<159>";
	var t_3=t_event;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<160>";
	if(t_3==7){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<161>";
		this.p_KeyEvent(1,384+t_data);
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<162>";
		if(t_3==8){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<163>";
			this.p_KeyEvent(2,384+t_data);
			pop_err();
			return;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<165>";
			if(t_3==9){
			}else{
				pop_err();
				return;
			}
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<169>";
	dbg_array(this.m__touchX,t_data)[dbg_index]=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<170>";
	dbg_array(this.m__touchY,t_data)[dbg_index]=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<171>";
	if(t_data==0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<172>";
		this.m__mouseX=t_x;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<173>";
		this.m__mouseY=t_y;
	}
	pop_err();
}
c_InputDevice.prototype.p_MotionEvent=function(t_event,t_data,t_x,t_y,t_z){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<178>";
	var t_4=t_event;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<179>";
	if(t_4==10){
	}else{
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<183>";
	this.m__accelX=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<184>";
	this.m__accelY=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<185>";
	this.m__accelZ=t_z;
	pop_err();
}
c_InputDevice.prototype.p_Reset=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<32>";
	for(var t_i=0;t_i<512;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<33>";
		dbg_array(this.m__keyDown,t_i)[dbg_index]=false;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<34>";
		dbg_array(this.m__keyHit,t_i)[dbg_index]=0;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<36>";
	this.m__keyHitPut=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<37>";
	this.m__charGet=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<38>";
	this.m__charPut=0;
	pop_err();
}
c_InputDevice.prototype.p_KeyDown=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<47>";
	if(t_key>0 && t_key<512){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<47>";
		pop_err();
		return dbg_array(this.m__keyDown,t_key)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<48>";
	pop_err();
	return false;
}
c_InputDevice.prototype.p_TouchX=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<77>";
	if(t_index>=0 && t_index<32){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<77>";
		pop_err();
		return dbg_array(this.m__touchX,t_index)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<78>";
	pop_err();
	return 0.0;
}
c_InputDevice.prototype.p_TouchY=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<82>";
	if(t_index>=0 && t_index<32){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<82>";
		pop_err();
		return dbg_array(this.m__touchY,t_index)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<83>";
	pop_err();
	return 0.0;
}
c_InputDevice.prototype.p_KeyHit=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<52>";
	if(t_key>0 && t_key<512){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<52>";
		pop_err();
		return dbg_array(this.m__keyHit,t_key)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<53>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/inputdevice.monkey<14>";
	pop_err();
	return this;
}
var bb_input_device=null;
function bb_input_SetInputDevice(t_dev){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<22>";
	bb_input_device=t_dev;
	pop_err();
	return 0;
}
var bb_app__devWidth=0;
var bb_app__devHeight=0;
function bb_app_ValidateDeviceWindow(t_notifyApp){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<57>";
	var t_w=bb_app__game.GetDeviceWidth();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<58>";
	var t_h=bb_app__game.GetDeviceHeight();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<59>";
	if(t_w==bb_app__devWidth && t_h==bb_app__devHeight){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<60>";
	bb_app__devWidth=t_w;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<61>";
	bb_app__devHeight=t_h;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<62>";
	if(t_notifyApp){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<62>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<192>";
	this.m__width=t_width;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<193>";
	this.m__height=t_height;
	pop_err();
	return this;
}
c_DisplayMode.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<189>";
	pop_err();
	return this;
}
function c_Map(){
	Object.call(this);
	this.m_root=null;
}
c_Map.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<7>";
	pop_err();
	return this;
}
c_Map.prototype.p_Compare=function(t_lhs,t_rhs){
}
c_Map.prototype.p_FindNode=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<157>";
	var t_node=this.m_root;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<159>";
	while((t_node)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<160>";
		var t_cmp=this.p_Compare(t_key,dbg_object(t_node).m_key);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<161>";
		if(t_cmp>0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<162>";
			t_node=dbg_object(t_node).m_right;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<163>";
			if(t_cmp<0){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<164>";
				t_node=dbg_object(t_node).m_left;
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<166>";
				pop_err();
				return t_node;
			}
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<169>";
	pop_err();
	return t_node;
}
c_Map.prototype.p_Contains=function(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<25>";
	var t_=this.p_FindNode(t_key)!=null;
	pop_err();
	return t_;
}
c_Map.prototype.p_RotateLeft=function(t_node){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<251>";
	var t_child=dbg_object(t_node).m_right;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<252>";
	dbg_object(t_node).m_right=dbg_object(t_child).m_left;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<253>";
	if((dbg_object(t_child).m_left)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<254>";
		dbg_object(dbg_object(t_child).m_left).m_parent=t_node;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<256>";
	dbg_object(t_child).m_parent=dbg_object(t_node).m_parent;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<257>";
	if((dbg_object(t_node).m_parent)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<258>";
		if(t_node==dbg_object(dbg_object(t_node).m_parent).m_left){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<259>";
			dbg_object(dbg_object(t_node).m_parent).m_left=t_child;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<261>";
			dbg_object(dbg_object(t_node).m_parent).m_right=t_child;
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<264>";
		this.m_root=t_child;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<266>";
	dbg_object(t_child).m_left=t_node;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<267>";
	dbg_object(t_node).m_parent=t_child;
	pop_err();
	return 0;
}
c_Map.prototype.p_RotateRight=function(t_node){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<271>";
	var t_child=dbg_object(t_node).m_left;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<272>";
	dbg_object(t_node).m_left=dbg_object(t_child).m_right;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<273>";
	if((dbg_object(t_child).m_right)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<274>";
		dbg_object(dbg_object(t_child).m_right).m_parent=t_node;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<276>";
	dbg_object(t_child).m_parent=dbg_object(t_node).m_parent;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<277>";
	if((dbg_object(t_node).m_parent)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<278>";
		if(t_node==dbg_object(dbg_object(t_node).m_parent).m_right){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<279>";
			dbg_object(dbg_object(t_node).m_parent).m_right=t_child;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<281>";
			dbg_object(dbg_object(t_node).m_parent).m_left=t_child;
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<284>";
		this.m_root=t_child;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<286>";
	dbg_object(t_child).m_right=t_node;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<287>";
	dbg_object(t_node).m_parent=t_child;
	pop_err();
	return 0;
}
c_Map.prototype.p_InsertFixup=function(t_node){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<212>";
	while(((dbg_object(t_node).m_parent)!=null) && dbg_object(dbg_object(t_node).m_parent).m_color==-1 && ((dbg_object(dbg_object(t_node).m_parent).m_parent)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<213>";
		if(dbg_object(t_node).m_parent==dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_left){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<214>";
			var t_uncle=dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_right;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<215>";
			if(((t_uncle)!=null) && dbg_object(t_uncle).m_color==-1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<216>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<217>";
				dbg_object(t_uncle).m_color=1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<218>";
				dbg_object(dbg_object(t_uncle).m_parent).m_color=-1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<219>";
				t_node=dbg_object(t_uncle).m_parent;
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<221>";
				if(t_node==dbg_object(dbg_object(t_node).m_parent).m_right){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<222>";
					t_node=dbg_object(t_node).m_parent;
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<223>";
					this.p_RotateLeft(t_node);
				}
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<225>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<226>";
				dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_color=-1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<227>";
				this.p_RotateRight(dbg_object(dbg_object(t_node).m_parent).m_parent);
			}
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<230>";
			var t_uncle2=dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_left;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<231>";
			if(((t_uncle2)!=null) && dbg_object(t_uncle2).m_color==-1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<232>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<233>";
				dbg_object(t_uncle2).m_color=1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<234>";
				dbg_object(dbg_object(t_uncle2).m_parent).m_color=-1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<235>";
				t_node=dbg_object(t_uncle2).m_parent;
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<237>";
				if(t_node==dbg_object(dbg_object(t_node).m_parent).m_left){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<238>";
					t_node=dbg_object(t_node).m_parent;
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<239>";
					this.p_RotateRight(t_node);
				}
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<241>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<242>";
				dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_color=-1;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<243>";
				this.p_RotateLeft(dbg_object(dbg_object(t_node).m_parent).m_parent);
			}
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<247>";
	dbg_object(this.m_root).m_color=1;
	pop_err();
	return 0;
}
c_Map.prototype.p_Set=function(t_key,t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<29>";
	var t_node=this.m_root;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<30>";
	var t_parent=null;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<30>";
	var t_cmp=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<32>";
	while((t_node)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<33>";
		t_parent=t_node;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<34>";
		t_cmp=this.p_Compare(t_key,dbg_object(t_node).m_key);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<35>";
		if(t_cmp>0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<36>";
			t_node=dbg_object(t_node).m_right;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<37>";
			if(t_cmp<0){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<38>";
				t_node=dbg_object(t_node).m_left;
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<40>";
				dbg_object(t_node).m_value=t_value;
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<41>";
				pop_err();
				return false;
			}
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<45>";
	t_node=c_Node.m_new.call(new c_Node,t_key,t_value,-1,t_parent);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<47>";
	if((t_parent)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<48>";
		if(t_cmp>0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<49>";
			dbg_object(t_parent).m_right=t_node;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<51>";
			dbg_object(t_parent).m_left=t_node;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<53>";
		this.p_InsertFixup(t_node);
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<55>";
		this.m_root=t_node;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<57>";
	pop_err();
	return true;
}
c_Map.prototype.p_Insert=function(t_key,t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<146>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<534>";
	c_Map.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<534>";
	pop_err();
	return this;
}
c_IntMap.prototype.p_Compare=function(t_lhs,t_rhs){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<537>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<13>";
	dbg_object(this).m_data=t_data.slice(0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<14>";
	dbg_object(this).m_length=t_data.length;
	pop_err();
	return this;
}
c_Stack.prototype.p_Push=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<71>";
	if(this.m_length==this.m_data.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<72>";
		this.m_data=resize_object_array(this.m_data,this.m_length*2+10);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<74>";
	dbg_array(this.m_data,this.m_length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<75>";
	this.m_length+=1;
	pop_err();
}
c_Stack.prototype.p_Push2=function(t_values,t_offset,t_count){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<83>";
	for(var t_i=0;t_i<t_count;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<84>";
		this.p_Push(dbg_array(t_values,t_offset+t_i)[dbg_index]);
	}
	pop_err();
}
c_Stack.prototype.p_Push3=function(t_values,t_offset){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<79>";
	this.p_Push2(t_values,t_offset,t_values.length-t_offset);
	pop_err();
}
c_Stack.prototype.p_ToArray=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<18>";
	var t_t=new_object_array(this.m_length);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<19>";
	for(var t_i=0;t_i<this.m_length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<20>";
		dbg_array(t_t,t_i)[dbg_index]=dbg_array(this.m_data,t_i)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/stack.monkey<22>";
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
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<364>";
	dbg_object(this).m_key=t_key;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<365>";
	dbg_object(this).m_value=t_value;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<366>";
	dbg_object(this).m_color=t_color;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<367>";
	dbg_object(this).m_parent=t_parent;
	pop_err();
	return this;
}
c_Node.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/map.monkey<361>";
	pop_err();
	return this;
}
var bb_app__displayModes=[];
var bb_app__desktopMode=null;
function bb_app_DeviceWidth(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<263>";
	pop_err();
	return bb_app__devWidth;
}
function bb_app_DeviceHeight(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<267>";
	pop_err();
	return bb_app__devHeight;
}
function bb_app_EnumDisplayModes(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<33>";
	var t_modes=bb_app__game.GetDisplayModes();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<34>";
	var t_mmap=c_IntMap.m_new.call(new c_IntMap);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<35>";
	var t_mstack=c_Stack.m_new.call(new c_Stack);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<36>";
	for(var t_i=0;t_i<t_modes.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<37>";
		var t_w=dbg_object(dbg_array(t_modes,t_i)[dbg_index]).width;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<38>";
		var t_h=dbg_object(dbg_array(t_modes,t_i)[dbg_index]).height;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<39>";
		var t_size=t_w<<16|t_h;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<40>";
		if(t_mmap.p_Contains(t_size)){
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<42>";
			var t_mode=c_DisplayMode.m_new.call(new c_DisplayMode,dbg_object(dbg_array(t_modes,t_i)[dbg_index]).width,dbg_object(dbg_array(t_modes,t_i)[dbg_index]).height);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<43>";
			t_mmap.p_Insert(t_size,t_mode);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<44>";
			t_mstack.p_Push(t_mode);
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<47>";
	bb_app__displayModes=t_mstack.p_ToArray();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<48>";
	var t_mode2=bb_app__game.GetDesktopMode();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<49>";
	if((t_mode2)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<50>";
		bb_app__desktopMode=c_DisplayMode.m_new.call(new c_DisplayMode,dbg_object(t_mode2).width,dbg_object(t_mode2).height);
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<52>";
		bb_app__desktopMode=c_DisplayMode.m_new.call(new c_DisplayMode,bb_app_DeviceWidth(),bb_app_DeviceHeight());
	}
	pop_err();
}
var bb_graphics_renderDevice=null;
function bb_graphics_SetMatrix(t_ix,t_iy,t_jx,t_jy,t_tx,t_ty){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<312>";
	dbg_object(bb_graphics_context).m_ix=t_ix;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<313>";
	dbg_object(bb_graphics_context).m_iy=t_iy;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<314>";
	dbg_object(bb_graphics_context).m_jx=t_jx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<315>";
	dbg_object(bb_graphics_context).m_jy=t_jy;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<316>";
	dbg_object(bb_graphics_context).m_tx=t_tx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<317>";
	dbg_object(bb_graphics_context).m_ty=t_ty;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<318>";
	dbg_object(bb_graphics_context).m_tformed=((t_ix!=1.0 || t_iy!=0.0 || t_jx!=0.0 || t_jy!=1.0 || t_tx!=0.0 || t_ty!=0.0)?1:0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<319>";
	dbg_object(bb_graphics_context).m_matDirty=1;
	pop_err();
	return 0;
}
function bb_graphics_SetMatrix2(t_m){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<308>";
	bb_graphics_SetMatrix(dbg_array(t_m,0)[dbg_index],dbg_array(t_m,1)[dbg_index],dbg_array(t_m,2)[dbg_index],dbg_array(t_m,3)[dbg_index],dbg_array(t_m,4)[dbg_index],dbg_array(t_m,5)[dbg_index]);
	pop_err();
	return 0;
}
function bb_graphics_SetColor(t_r,t_g,t_b){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<254>";
	dbg_object(bb_graphics_context).m_color_r=t_r;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<255>";
	dbg_object(bb_graphics_context).m_color_g=t_g;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<256>";
	dbg_object(bb_graphics_context).m_color_b=t_b;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<257>";
	bb_graphics_renderDevice.SetColor(t_r,t_g,t_b);
	pop_err();
	return 0;
}
function bb_graphics_SetAlpha(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<271>";
	dbg_object(bb_graphics_context).m_alpha=t_alpha;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<272>";
	bb_graphics_renderDevice.SetAlpha(t_alpha);
	pop_err();
	return 0;
}
function bb_graphics_SetBlend(t_blend){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<280>";
	dbg_object(bb_graphics_context).m_blend=t_blend;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<281>";
	bb_graphics_renderDevice.SetBlend(t_blend);
	pop_err();
	return 0;
}
function bb_graphics_SetScissor(t_x,t_y,t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<289>";
	dbg_object(bb_graphics_context).m_scissor_x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<290>";
	dbg_object(bb_graphics_context).m_scissor_y=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<291>";
	dbg_object(bb_graphics_context).m_scissor_width=t_width;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<292>";
	dbg_object(bb_graphics_context).m_scissor_height=t_height;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<293>";
	bb_graphics_renderDevice.SetScissor(((t_x)|0),((t_y)|0),((t_width)|0),((t_height)|0));
	pop_err();
	return 0;
}
function bb_graphics_BeginRender(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<225>";
	bb_graphics_renderDevice=bb_graphics_device;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<226>";
	dbg_object(bb_graphics_context).m_matrixSp=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<227>";
	bb_graphics_SetMatrix(1.0,0.0,0.0,1.0,0.0,0.0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<228>";
	bb_graphics_SetColor(255.0,255.0,255.0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<229>";
	bb_graphics_SetAlpha(1.0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<230>";
	bb_graphics_SetBlend(0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<231>";
	bb_graphics_SetScissor(0.0,0.0,(bb_app_DeviceWidth()),(bb_app_DeviceHeight()));
	pop_err();
	return 0;
}
function bb_graphics_EndRender(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<235>";
	bb_graphics_renderDevice=null;
	pop_err();
	return 0;
}
function c_BBGameEvent(){
	Object.call(this);
}
function bb_app_EndApp(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<259>";
	error("");
	pop_err();
}
function c_iObject(){
	Object.call(this);
	this.m__enabled=1;
	this.m__tattoo=false;
	this.m__inPool=0;
}
c_iObject.prototype.p_OnDisabled=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<153>";
	pop_err();
	return 0;
}
c_iObject.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<20>";
	pop_err();
	return this;
}
c_iObject.prototype.p_OnDisable=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<146>";
	pop_err();
	return 0;
}
c_iObject.prototype.p_Disable=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<69>";
	this.m__enabled=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/pooling.monkey<71>";
	this.p_OnDisable();
	pop_err();
}
function c_iScene(){
	c_iObject.call(this);
	this.m__visible=1;
	this.m__started=false;
	this.m__cold=true;
	this.m__paused=false;
}
c_iScene.prototype=extend_class(c_iObject);
c_iScene.prototype.p_OnBack=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<402>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnClose=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<415>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnLoading=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<440>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<472>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<540>";
	this.p_OnRender();
	pop_err();
}
c_iScene.prototype.p_OnResize=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<483>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Resume=function(){
	push_err();
	pop_err();
}
c_iScene.prototype.p_OnResume=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<489>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Suspend=function(){
	push_err();
	pop_err();
}
c_iScene.prototype.p_OnSuspend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<514>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnStop=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<503>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Set2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<569>";
	bb_app2_iCurrentScene=this;
	pop_err();
}
c_iScene.prototype.p_OnColdStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<424>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<496>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnPaused=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<456>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<521>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<598>";
	if((bb_app2_iNextScene)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<600>";
		if(this.m__started){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<601>";
			bb_input_ResetInput();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<602>";
			this.m__started=false;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<603>";
			this.p_OnStop();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<606>";
		bb_app2_iNextScene.p_Set2();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<608>";
		bb_app2_iNextScene=null;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<610>";
		if(dbg_object(bb_app2_iCurrentScene).m__cold==true){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<611>";
			bb_app2_iCurrentScene.p_OnColdStart();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<612>";
			dbg_object(bb_app2_iCurrentScene).m__cold=false;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<615>";
		if(dbg_object(bb_app2_iCurrentScene).m__started==false){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<616>";
			bb_app2_iCurrentScene.p_OnStart();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<617>";
			dbg_object(bb_app2_iCurrentScene).m__started=true;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<620>";
		if(dbg_object(bb_app2_iCurrentScene).m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<622>";
			if(dbg_object(bb_app2_iCurrentScene).m__paused){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<623>";
				bb_app2_iCurrentScene.p_OnPaused();
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<625>";
				bb_app2_iCurrentScene.p_OnUpdate();
			}
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<630>";
			if(dbg_object(bb_app2_iCurrentScene).m__enabled>1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<631>";
				bb_app2_iCurrentScene.p_OnDisabled();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<632>";
				dbg_object(bb_app2_iCurrentScene).m__enabled=dbg_object(bb_app2_iCurrentScene).m__enabled-1;
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<639>";
		if(dbg_object(bb_app2_iCurrentScene).m__cold==true){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<640>";
			bb_app2_iCurrentScene.p_OnColdStart();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<641>";
			dbg_object(bb_app2_iCurrentScene).m__cold=false;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<644>";
		if(dbg_object(bb_app2_iCurrentScene).m__started==false){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<645>";
			bb_app2_iCurrentScene.p_OnStart();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<646>";
			dbg_object(bb_app2_iCurrentScene).m__started=true;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<649>";
		if(dbg_object(bb_app2_iCurrentScene).m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<651>";
			if(dbg_object(bb_app2_iCurrentScene).m__paused){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<652>";
				bb_app2_iCurrentScene.p_OnPaused();
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<654>";
				bb_app2_iCurrentScene.p_OnUpdate();
			}
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<659>";
			if(dbg_object(bb_app2_iCurrentScene).m__enabled>1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<660>";
				bb_app2_iCurrentScene.p_OnDisabled();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<661>";
				dbg_object(bb_app2_iCurrentScene).m__enabled=dbg_object(bb_app2_iCurrentScene).m__enabled-1;
			}
		}
	}
	pop_err();
}
c_iScene.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<433>";
	pop_err();
	return 0;
}
c_iScene.prototype.p_SystemInit=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<587>";
	this.p_Set2();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<589>";
	this.p_OnCreate();
	pop_err();
}
c_iScene.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<388>";
	c_iObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<389>";
	this.p_SystemInit();
	pop_err();
	return this;
}
var bb_app2_iCurrentScene=null;
function bb_app2_iBack(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<46>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m__enabled)!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<46>";
		bb_app2_iCurrentScene.p_OnBack();
	}
	pop_err();
}
function bb_app2_iClose(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<64>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m__enabled)!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<64>";
		bb_app2_iCurrentScene.p_OnClose();
	}
	pop_err();
}
function bb_app2_iLoading(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<99>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m__enabled)!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<99>";
		bb_app2_iCurrentScene.p_OnLoading();
	}
	pop_err();
}
function bb_app2_iRender(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<143>";
	if((bb_app2_iCurrentScene)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<145>";
		if((dbg_object(bb_app2_iCurrentScene).m__enabled)!=0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<147>";
			if(dbg_object(bb_app2_iCurrentScene).m__visible==1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<148>";
				bb_app2_iCurrentScene.p_Render();
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<150>";
				if(dbg_object(bb_app2_iCurrentScene).m__visible>1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<150>";
					dbg_object(bb_app2_iCurrentScene).m__visible=dbg_object(bb_app2_iCurrentScene).m__visible-1;
				}
			}
		}
	}
	pop_err();
}
function bb_app2_iResize(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<185>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m__enabled)!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<185>";
		bb_app2_iCurrentScene.p_OnResize();
	}
	pop_err();
}
function bb_app2_iResume(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<203>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m__enabled)!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<204>";
		bb_app2_iCurrentScene.p_Resume();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<205>";
		bb_app2_iCurrentScene.p_OnResume();
	}
	pop_err();
}
function bb_app2_iSuspend(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<249>";
	if(((bb_app2_iCurrentScene)!=null) && ((dbg_object(bb_app2_iCurrentScene).m__enabled)!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<250>";
		bb_app2_iCurrentScene.p_Suspend();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<251>";
		bb_app2_iCurrentScene.p_OnSuspend();
	}
	pop_err();
}
function c_iDeltaTimer(){
	Object.call(this);
	this.m__targetFPS=60.0;
	this.m__lastTicks=0;
	this.m__frameTime=.0;
	this.m__elapsedTime=0.0;
	this.m__timeScale=1.0;
	this.m__deltaTime=.0;
	this.m__elapsedDelta=0.0;
}
c_iDeltaTimer.m_new=function(t_targetFPS){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<24>";
	this.m__targetFPS=t_targetFPS;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<25>";
	this.m__lastTicks=bb_app_Millisecs();
	pop_err();
	return this;
}
c_iDeltaTimer.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<11>";
	pop_err();
	return this;
}
c_iDeltaTimer.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<94>";
	this.m__frameTime=(bb_app_Millisecs()-this.m__lastTicks);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<95>";
	this.m__lastTicks=bb_app_Millisecs();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<96>";
	this.m__elapsedTime+=this.m__frameTime;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<97>";
	this.m__deltaTime=this.m__frameTime/(1000.0/this.m__targetFPS)*this.m__timeScale;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<98>";
	this.m__elapsedDelta=this.m__elapsedDelta+this.m__deltaTime/this.m__targetFPS;
	pop_err();
}
c_iDeltaTimer.prototype.p_Resume=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/deltatimer.monkey<57>";
	this.m__lastTicks=bb_app_Millisecs();
	pop_err();
}
function bb_app_Millisecs(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<233>";
	var t_=bb_app__game.Millisecs();
	pop_err();
	return t_;
}
var bb_app2_iDT=null;
var bb_app2_iSpeed=0;
var bb_app2_iNextScene=null;
function bb_input_ResetInput(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<26>";
	bb_input_device.p_Reset();
	pop_err();
	return 0;
}
function bb_app2_iUpdate(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<271>";
	bb_app2_iDT.p_Update();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<273>";
	if((bb_app2_iCurrentScene)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<274>";
		if(bb_app2_iSpeed==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<275>";
			bb_app2_iCurrentScene.p_Update();
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<277>";
			for(var t_i=0;t_i<bb_app2_iSpeed;t_i=t_i+1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<278>";
				bb_app2_iCurrentScene.p_Update();
			}
		}
	}
	pop_err();
}
var bb_random_Seed=0;
function c_iEngine(){
	c_iScene.call(this);
	this.m__timeCode=0;
	this.m__autoCls=true;
	this.m__clsRed=.0;
	this.m__colorFade=1.0;
	this.m__clsGreen=.0;
	this.m__clsBlue=.0;
	this.m__renderToPlayfield=true;
	this.m__playfieldList=c_iList.m_new.call(new c_iList);
	this.m__currentPlayfield=null;
	this.m__alphaFade=1.0;
	this.m__borders=true;
	this.m__showSystemGui=false;
	this.m__taskList=c_iList5.m_new.call(new c_iList5);
	this.m__countDown=0;
	this.m__scoreCount=0;
	this.m__playfieldPointer=null;
	this.m__collisionReadList=c_iList3.m_new.call(new c_iList3);
	this.m__layerPointer=null;
	this.m__collisionWriteList=c_iList3.m_new.call(new c_iList3);
}
c_iEngine.prototype=extend_class(c_iScene);
c_iEngine.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<13>";
	c_iScene.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<13>";
	pop_err();
	return this;
}
c_iEngine.prototype.p_OnColdStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<381>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_OnResize=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<397>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_AlphaFade=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<139>";
	pop_err();
	return this.m__alphaFade;
}
c_iEngine.prototype.p_AlphaFade2=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<146>";
	this.m__alphaFade=t_alpha;
	pop_err();
}
c_iEngine.prototype.p_SetAlpha=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<626>";
	if((this.m__currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<627>";
		this.m__currentPlayfield.p_SetAlpha(t_alpha);
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<629>";
		bb_graphics_SetAlpha(t_alpha*bb_globals_iEnginePointer.p_AlphaFade());
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<632>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_ColorFade=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<194>";
	pop_err();
	return this.m__colorFade;
}
c_iEngine.prototype.p_ColorFade2=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<201>";
	this.m__colorFade=t_value;
	pop_err();
}
c_iEngine.prototype.p_SetColor=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<641>";
	if((this.m__currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<642>";
		this.m__currentPlayfield.p_SetColor(t_red,t_green,t_blue);
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<644>";
		bb_graphics_SetColor(t_red*bb_globals_iEnginePointer.p_ColorFade(),t_green*bb_globals_iEnginePointer.p_ColorFade(),t_blue*bb_globals_iEnginePointer.p_ColorFade());
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<647>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_OnTopRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<406>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_RenderPlayfields=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<563>";
	this.m__currentPlayfield=this.m__playfieldList.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<565>";
	while((this.m__currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<567>";
		if(dbg_object(this.m__currentPlayfield).m__enabled==1 && dbg_object(this.m__currentPlayfield).m__visible){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<567>";
			this.m__currentPlayfield.p_Render();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<569>";
		this.m__currentPlayfield=this.m__playfieldList.p_Ascend();
	}
	pop_err();
}
c_iEngine.prototype.p_RenderSystemGui=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<580>";
	if(this.m__showSystemGui && ((c_iSystemGui.m__playfield)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<582>";
		c_iStack2D.m_Push();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<584>";
		bb_graphics_Scale(dbg_object(c_iSystemGui.m__playfield).m__scaleX,dbg_object(c_iSystemGui.m__playfield).m__scaleY);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<586>";
		bb_graphics_Translate(c_iSystemGui.m__playfield.p_PositionX()/dbg_object(c_iSystemGui.m__playfield).m__scaleX,c_iSystemGui.m__playfield.p_PositionY()/dbg_object(c_iSystemGui.m__playfield).m__scaleY);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<588>";
		var t_g=dbg_array(dbg_object(c_iSystemGui.m__playfield).m__guiList,dbg_object(c_iSystemGui.m__playfield).m__guiPage)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<589>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<590>";
			if(dbg_object(t_g).m__enabled==1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<591>";
				if(dbg_object(t_g).m__ghost){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<591>";
					t_g.p_RenderGhost();
				}else{
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<591>";
					t_g.p_Render();
				}
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<592>";
				t_g=dbg_array(dbg_object(c_iSystemGui.m__playfield).m__guiList,dbg_object(c_iSystemGui.m__playfield).m__guiPage)[dbg_index].p_Ascend();
			}
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<596>";
		c_iStack2D.m_Pop();
	}
	pop_err();
}
c_iEngine.prototype.p_DebugRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<217>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<501>";
	if((bb_functions_iTimeCode())!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<503>";
		if(this.m__autoCls){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<503>";
			bb_gfx_iCls(this.m__clsRed*this.m__colorFade,this.m__clsGreen*this.m__colorFade,this.m__clsBlue*this.m__colorFade);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<505>";
		if(this.m__renderToPlayfield==false || this.m__playfieldList.p_Length()==0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<507>";
			c_iStack2D.m_Push();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<509>";
			this.p_SetAlpha(1.0);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<510>";
			this.p_SetColor(255.0,255.0,255.0);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<512>";
			this.p_OnRender();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<514>";
			c_iStack2D.m_Pop();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<518>";
		this.p_RenderPlayfields();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<520>";
		if(this.m__renderToPlayfield==false || this.m__playfieldList.p_Length()==0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<522>";
			c_iStack2D.m_Push();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<524>";
			this.p_SetAlpha(1.0);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<525>";
			this.p_SetColor(255.0,255.0,255.0);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<527>";
			this.p_OnTopRender();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<529>";
			c_iStack2D.m_Pop();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<533>";
		this.p_RenderSystemGui();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<535>";
		c_iTask.m_Render(this.m__taskList);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<537>";
		c_iTask.m_Render(bb_globals_iTaskList);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<539>";
		c_iStack2D.m_Push();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<541>";
		var t_a=this.m__alphaFade;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<542>";
		var t_c=this.m__colorFade;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<544>";
		this.m__alphaFade=1.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<545>";
		this.m__colorFade=1.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<547>";
		this.p_DebugRender();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<549>";
		this.m__alphaFade=t_a;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<550>";
		this.m__colorFade=t_c;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<552>";
		c_iStack2D.m_Pop();
	}
	pop_err();
}
c_iEngine.prototype.p_Resume=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<603>";
	bb_app2_iDT.p_Resume();
	pop_err();
}
c_iEngine.prototype.p_Set2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<617>";
	bb_app2_iCurrentScene=(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<618>";
	bb_globals_iEnginePointer=this;
	pop_err();
}
c_iEngine.prototype.p_OnGuiPageChange=function(t_playfield){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<388>";
	pop_err();
	return 0;
}
c_iEngine.prototype.p_UpdateSystemGui=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<867>";
	if(this.m__showSystemGui && ((c_iSystemGui.m__playfield)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<869>";
		if(dbg_object(c_iSystemGui.m__playfield).m__nextGuiPage>=0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<870>";
			dbg_object(c_iSystemGui.m__playfield).m__guiPage=dbg_object(c_iSystemGui.m__playfield).m__nextGuiPage;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<871>";
			dbg_object(c_iSystemGui.m__playfield).m__nextGuiPage=-1;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<874>";
		var t_g=null;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<876>";
		t_g=dbg_array(dbg_object(c_iSystemGui.m__playfield).m__guiList,dbg_object(c_iSystemGui.m__playfield).m__guiPage)[dbg_index].p_Last();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<877>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<879>";
			t_g.p_UpdateWorldXY();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<881>";
			if(dbg_object(t_g).m__enabled==1 && !dbg_object(t_g).m__ghost){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<881>";
				t_g.p_UpdateInput();
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<883>";
			t_g=dbg_array(dbg_object(c_iSystemGui.m__playfield).m__guiList,dbg_object(c_iSystemGui.m__playfield).m__guiPage)[dbg_index].p_Descend();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<887>";
		t_g=dbg_array(dbg_object(c_iSystemGui.m__playfield).m__guiList,dbg_object(c_iSystemGui.m__playfield).m__guiPage)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<888>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<890>";
			if(dbg_object(t_g).m__enabled==1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<892>";
				if(dbg_object(t_g).m__ghost){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<893>";
					t_g.p_UpdateGhost();
				}else{
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<895>";
					t_g.p_Update();
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<896>";
					t_g.p_OnUpdate();
				}
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<901>";
			t_g=dbg_array(dbg_object(c_iSystemGui.m__playfield).m__guiList,dbg_object(c_iSystemGui.m__playfield).m__guiPage)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iEngine.prototype.p_UpdatePlayfields=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<819>";
	this.m__currentPlayfield=this.m__playfieldList.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<821>";
	while((this.m__currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<822>";
		if((dbg_object(this.m__currentPlayfield).m__enabled)!=0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<822>";
			this.m__currentPlayfield.p_Update();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<823>";
		this.m__currentPlayfield=this.m__playfieldList.p_Ascend();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<826>";
	this.m__currentPlayfield=this.m__playfieldList.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<828>";
	while((this.m__currentPlayfield)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<830>";
		if((dbg_object(this.m__currentPlayfield).m__enabled)!=0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<832>";
			dbg_object(this.m__currentPlayfield).m__currentLayer=dbg_object(this.m__currentPlayfield).m__layerList.p_First();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<834>";
			while((dbg_object(this.m__currentPlayfield).m__currentLayer)!=null){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<836>";
				if(((dbg_object(dbg_object(this.m__currentPlayfield).m__currentLayer).m__enabled)!=0) && dbg_object(dbg_object(this.m__currentPlayfield).m__currentLayer).m__lastUpdate!=this.m__timeCode){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<838>";
					dbg_object(dbg_object(this.m__currentPlayfield).m__currentLayer).m__lastUpdate=this.m__timeCode;
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<840>";
					dbg_object(this.m__currentPlayfield).m__currentLayer.p_Update();
				}
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<844>";
				dbg_object(this.m__currentPlayfield).m__currentLayer=dbg_object(this.m__currentPlayfield).m__layerList.p_Ascend();
			}
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<850>";
		this.m__currentPlayfield=this.m__playfieldList.p_Ascend();
	}
	pop_err();
}
c_iEngine.prototype.p_UpdateService=function(){
	push_err();
	pop_err();
}
c_iEngine.prototype.p_UpdateCollisions=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<757>";
	var t_s1=this.m__collisionReadList.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<758>";
	while((t_s1)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<760>";
		if(((t_s1.p_Layer())!=null) && dbg_object(t_s1.p_Layer()).m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<762>";
			var t_s2=this.m__collisionWriteList.p_First();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<763>";
			while((t_s2)!=null){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<765>";
				if(((t_s2.p_Layer())!=null) && dbg_object(t_s2.p_Layer()).m__enabled==1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<766>";
					if((t_s1.p_Collides(t_s2))!=0){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<766>";
						t_s1.p_OnCollision(t_s2);
					}
				}
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<769>";
				t_s2=this.m__collisionWriteList.p_Ascend();
			}
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<775>";
		t_s1=this.m__collisionReadList.p_Ascend();
	}
	pop_err();
}
c_iEngine.prototype.p_UpdateLogic=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<786>";
	if(this.m__enabled==1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<788>";
		this.p_UpdatePlayfields();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<790>";
		this.p_UpdateSystemGui();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<792>";
		this.p_UpdateService();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<794>";
		this.p_OnUpdate();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<796>";
		this.p_UpdateCollisions();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<798>";
		this.m__timeCode=this.m__timeCode+1;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<802>";
		if(this.m__enabled>1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<804>";
			this.p_OnDisabled();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<806>";
			this.m__enabled=this.m__enabled-1;
		}
	}
	pop_err();
}
c_iEngine.prototype.p_UpdateFixed=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<914>";
	if(this.m__paused){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<916>";
		this.p_UpdateSystemGui();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<917>";
		this.p_OnPaused();
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<921>";
		this.p_UpdateLogic();
	}
	pop_err();
}
c_iEngine.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<688>";
	c_iGuiObject.m__topObject=null;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<690>";
	if((bb_app2_iNextScene)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<692>";
		if(this.m__started){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<693>";
			bb_input_ResetInput();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<694>";
			this.m__started=false;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<695>";
			this.p_OnStop();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<698>";
		bb_app2_iNextScene.p_Set2();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<700>";
		bb_app2_iNextScene=null;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<702>";
		dbg_object(bb_globals_iEnginePointer).m__alphaFade=1.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<703>";
		dbg_object(bb_globals_iEnginePointer).m__colorFade=1.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<704>";
		dbg_object(bb_globals_iEnginePointer).m__countDown=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<705>";
		dbg_object(bb_globals_iEnginePointer).m__paused=false;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<706>";
		dbg_object(bb_globals_iEnginePointer).m__scoreCount=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<707>";
		dbg_object(bb_globals_iEnginePointer).m__timeCode=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<709>";
		if((c_iSystemGui.m__playfield)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<709>";
			c_iSystemGui.m_GuiPage(0);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<711>";
		if(dbg_object(bb_globals_iEnginePointer).m__cold==true){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<712>";
			bb_globals_iEnginePointer.p_OnColdStart();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<713>";
			dbg_object(bb_globals_iEnginePointer).m__cold=false;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<716>";
		if(dbg_object(bb_globals_iEnginePointer).m__started==false){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<717>";
			bb_globals_iEnginePointer.p_OnStart();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<718>";
			dbg_object(bb_globals_iEnginePointer).m__started=true;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<721>";
		if(dbg_object(bb_globals_iEnginePointer).m__paused){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<722>";
			bb_globals_iEnginePointer.p_UpdateSystemGui();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<723>";
			bb_globals_iEnginePointer.p_OnPaused();
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<725>";
			bb_globals_iEnginePointer.p_UpdateLogic();
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<730>";
		if(dbg_object(bb_globals_iEnginePointer).m__cold==true){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<731>";
			bb_globals_iEnginePointer.p_OnColdStart();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<732>";
			dbg_object(bb_globals_iEnginePointer).m__cold=false;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<735>";
		if(dbg_object(bb_globals_iEnginePointer).m__started==false){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<736>";
			bb_globals_iEnginePointer.p_OnStart();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<737>";
			dbg_object(bb_globals_iEnginePointer).m__started=true;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<740>";
		bb_globals_iEnginePointer.p_UpdateFixed();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<744>";
	c_iTask.m_Update(this.m__taskList);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<746>";
	c_iTask.m_Update(bb_globals_iTaskList);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engine.monkey<748>";
	c_iEngineObject.m_DestroyList2();
	pop_err();
}
function c_MenuScene(){
	c_iEngine.call(this);
}
c_MenuScene.prototype=extend_class(c_iEngine);
c_MenuScene.m_new=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<46>";
	c_iEngine.m_new.call(this);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<46>";
	pop_err();
	return this;
}
c_MenuScene.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<50>";
	print("Creating Menu");
	pop_err();
	return 0;
}
c_MenuScene.prototype.p_OnRender=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<56>";
	bb_graphics_DrawText("Press Enter to generate Cellularly",200.0,200.0,0.0,0.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<57>";
	bb_graphics_DrawText("Press Space to generate with Noise",200.0,300.0,0.0,0.0);
	pop_err();
	return 0;
}
c_MenuScene.prototype.p_OnStart=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<63>";
	print("Starting Menu");
	pop_err();
	return 0;
}
c_MenuScene.prototype.p_OnStop=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<69>";
	print("Stopping Menu");
	pop_err();
	return 0;
}
c_MenuScene.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<76>";
	if((bb_input_KeyHit(13))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<77>";
		print("Switch");
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<78>";
		bb_app2_iStart2(bb_main_gameplay);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<81>";
	if((bb_input_KeyHit(32))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<82>";
		print("Switch to Noise");
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<83>";
		bb_app2_iStart2(bb_main_noiseTest);
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
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<94>";
	c_iEngine.m_new.call(this);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<94>";
	pop_err();
	return this;
}
c_GameplayScene.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<107>";
	print("Creating Gameplay");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<108>";
	dbg_object(this).m_playfield=c_iPlayfield.m_new.call(new c_iPlayfield);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<109>";
	dbg_object(this).m_playfield.p_AttachLast();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<110>";
	dbg_object(this).m_playfield.p_AutoCls(0,0,0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<111>";
	dbg_object(this).m_playfield.p_Width2(600.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<112>";
	dbg_object(this).m_playfield.p_Height2(445.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<113>";
	dbg_object(this).m_playfield.p_Position(16.0,16.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<114>";
	dbg_object(this).m_playfield.p_ZoomPointX2(200.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<115>";
	dbg_object(this).m_playfield.p_ZoomPointY2(128.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<129>";
	dbg_object(this).m_layer=c_iLayer.m_new.call(new c_iLayer);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<133>";
	var t_img=bb_gfx_iLoadSprite2("char_walk_down.png",69,102,4);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<134>";
	dbg_object(this).m_layer.p_AttachLast3(dbg_object(this).m_playfield);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<138>";
	dbg_object(this).m_sprite1=c_iLayerSprite.m_new.call(new c_iLayerSprite);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<139>";
	dbg_object(this).m_sprite1.p_AttachLast4(dbg_object(this).m_layer);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<140>";
	dbg_object(this).m_sprite1.p_ImagePointer2(t_img);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<141>";
	dbg_object(this).m_sprite1.p_Position(300.0,275.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<149>";
	dbg_object(this).m_p1=c_Player.m_new.call(new c_Player,t_img,100,100);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<151>";
	this.m_music=bb_audio_LoadSound("tetris.mp3");
	pop_err();
	return 0;
}
c_GameplayScene.prototype.p_OnRender=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<159>";
	this.m_room.p_Draw();
	pop_err();
	return 0;
}
c_GameplayScene.prototype.p_OnStart=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<165>";
	print("Starting Gameplay");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<166>";
	this.m_room=c_Level.m_new.call(new c_Level,0,0,150,100,"Cellular");
	pop_err();
	return 0;
}
c_GameplayScene.prototype.p_OnStop=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<175>";
	print("Stopping Gameplay");
	pop_err();
	return 0;
}
c_GameplayScene.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<209>";
	if((bb_input_KeyDown(37))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<210>";
		dbg_object(this).m_playfield.p_CameraX2(dbg_object(this).m_playfield.p_CameraX()-4.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<213>";
	if((bb_input_KeyDown(39))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<214>";
		dbg_object(this).m_playfield.p_CameraX2(dbg_object(this).m_playfield.p_CameraX()+4.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<217>";
	if((bb_input_KeyDown(38))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<218>";
		dbg_object(this).m_playfield.p_CameraY2(dbg_object(this).m_playfield.p_CameraY()-4.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<221>";
	if((bb_input_KeyDown(40))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<222>";
		dbg_object(this).m_playfield.p_CameraY2(dbg_object(this).m_playfield.p_CameraY()+4.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<225>";
	this.m_sprite1.p_AnimationLoop(1.0,"60");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<226>";
	dbg_object(this).m_sprite1.p_Show("LOOPING ANIMATION:");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<227>";
	dbg_object(this).m_sprite1.p_Show("FramePointer="+String(dbg_object(this).m_sprite1.p_Frame2()));
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/main.monkey<229>";
	this.m_p1.p_Update();
	pop_err();
	return 0;
}
var bb_main_gameplay=null;
function c_NoiseTestScene(){
	c_iEngine.call(this);
	this.m_playfieldN=null;
	this.m_mapWidth=0;
	this.m_mapHeight=0;
	this.m_chunks=[];
	this.m_biomes=[];
	this.m_enemyPlacement=[];
	this.m_noiseMap=[];
	this.m_moisture=[];
	this.m_caves=new_object_array(1);
}
c_NoiseTestScene.prototype=extend_class(c_iEngine);
c_NoiseTestScene.m_new=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<12>";
	c_iEngine.m_new.call(this);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<12>";
	pop_err();
	return this;
}
c_NoiseTestScene.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<48>";
	print("Creating Noise Test");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<49>";
	dbg_object(this).m_playfieldN=c_iPlayfield.m_new.call(new c_iPlayfield);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<50>";
	dbg_object(this).m_playfieldN.p_AttachLast();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<51>";
	dbg_object(this).m_playfieldN.p_AutoCls(0,0,0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<52>";
	dbg_object(this).m_playfieldN.p_Width2(600.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<53>";
	dbg_object(this).m_playfieldN.p_Height2(460.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<54>";
	dbg_object(this).m_playfieldN.p_Position(25.0,15.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<55>";
	dbg_object(this).m_playfieldN.p_ZoomPointX2(200.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<56>";
	dbg_object(this).m_playfieldN.p_ZoomPointY2(128.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<58>";
	bb_noisetestscene_textures=bb_gfx_iLoadSprite2("textures20.png",20,20,9);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<59>";
	bb_noisetestscene_enemies=bb_gfx_iLoadSprite2("enemies20.png",20,20,9);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<66>";
	this.m_mapWidth=600;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<67>";
	this.m_mapHeight=460;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<69>";
	this.m_chunks=bb_noisetestscene_setArray(30,23);
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_drawNoiseMap=function(t_w,t_h){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<362>";
	var t_xOffset=((dbg_object(this).m_playfieldN.p_CameraX())|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<363>";
	var t_yOffset=((dbg_object(this).m_playfieldN.p_CameraY())|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<364>";
	var t_xTarget=t_xOffset+30;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<365>";
	var t_yTarget=t_yOffset+23;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<369>";
	for(var t_i=t_xOffset;t_i<t_xTarget;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<370>";
		for(var t_j=t_yOffset;t_j<t_yTarget;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<371>";
			bb_graphics_DrawImage(bb_noisetestscene_textures,((t_i-t_xOffset)*20+t_xOffset),((t_j-t_yOffset)*20+t_yOffset),dbg_array(dbg_array(this.m_biomes,t_i)[dbg_index],t_j)[dbg_index]);
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_drawEnemies=function(t_w,t_h){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<386>";
	var t_xOffset=((dbg_object(this).m_playfieldN.p_CameraX())|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<387>";
	var t_yOffset=((dbg_object(this).m_playfieldN.p_CameraY())|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<388>";
	var t_xTarget=t_xOffset+30;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<389>";
	var t_yTarget=t_yOffset+23;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<391>";
	for(var t_i=t_xOffset;t_i<t_xTarget;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<392>";
		for(var t_j=t_yOffset;t_j<t_yTarget;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<393>";
			if(!(dbg_array(dbg_array(this.m_enemyPlacement,t_i)[dbg_index],t_j)[dbg_index]==-1)){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<394>";
				bb_graphics_DrawImage(bb_noisetestscene_enemies,((t_i-t_xOffset)*20+t_xOffset),((t_j-t_yOffset)*20+t_yOffset),dbg_array(dbg_array(this.m_enemyPlacement,t_i)[dbg_index],t_j)[dbg_index]);
			}
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_OnRender=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<75>";
	this.p_drawNoiseMap(this.m_mapWidth,this.m_mapHeight);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<76>";
	this.p_drawEnemies(this.m_mapWidth,this.m_mapHeight);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<78>";
	bb_graphics_DrawText("CameraX: "+String(dbg_object(this).m_playfieldN.p_CameraX()),dbg_object(this).m_playfieldN.p_CameraX()+10.0,dbg_object(this).m_playfieldN.p_CameraY()+10.0,0.0,0.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<79>";
	bb_graphics_DrawText("CameraY: "+String(dbg_object(this).m_playfieldN.p_CameraY()),dbg_object(this).m_playfieldN.p_CameraX()+10.0,dbg_object(this).m_playfieldN.p_CameraY()+20.0,0.0,0.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<80>";
	bb_graphics_DrawText("MapX: "+String(dbg_object(this).m_playfieldN.p_CameraX()),dbg_object(this).m_playfieldN.p_CameraX()+10.0,dbg_object(this).m_playfieldN.p_CameraY()+40.0,0.0,0.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<81>";
	bb_graphics_DrawText("MapY: "+String(dbg_object(this).m_playfieldN.p_CameraY()),dbg_object(this).m_playfieldN.p_CameraX()+10.0,dbg_object(this).m_playfieldN.p_CameraY()+50.0,0.0,0.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<82>";
	bb_graphics_DrawText("MapXEnd: "+String(dbg_object(this).m_playfieldN.p_CameraX()+120.0),dbg_object(this).m_playfieldN.p_CameraX()+10.0,dbg_object(this).m_playfieldN.p_CameraY()+70.0,0.0,0.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<83>";
	bb_graphics_DrawText("MapYEnd: "+String(dbg_object(this).m_playfieldN.p_CameraY()+89.0),dbg_object(this).m_playfieldN.p_CameraX()+10.0,dbg_object(this).m_playfieldN.p_CameraY()+80.0,0.0,0.0);
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_determineBiomes=function(t_elevation,t_moist,t_width,t_height){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<404>";
	var t_localBiome=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<406>";
	for(var t_i=0;t_i<t_width;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<407>";
		for(var t_j=0;t_j<t_height;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<408>";
			if(dbg_array(dbg_array(this.m_noiseMap,t_i)[dbg_index],t_j)[dbg_index]<-0.35){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<409>";
				t_localBiome=0;
			}else{
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<410>";
				if(dbg_array(dbg_array(this.m_noiseMap,t_i)[dbg_index],t_j)[dbg_index]<-0.3){
					err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<411>";
					t_localBiome=1;
				}else{
					err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<412>";
					if(dbg_array(dbg_array(this.m_noiseMap,t_i)[dbg_index],t_j)[dbg_index]<-0.2){
						err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<413>";
						t_localBiome=2;
					}else{
						err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<414>";
						if(dbg_array(dbg_array(this.m_noiseMap,t_i)[dbg_index],t_j)[dbg_index]<-0.1){
							err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<415>";
							t_localBiome=3;
						}else{
							err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<416>";
							if(dbg_array(dbg_array(this.m_noiseMap,t_i)[dbg_index],t_j)[dbg_index]<0.2){
								err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<417>";
								t_localBiome=4;
							}else{
								err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<418>";
								if(dbg_array(dbg_array(this.m_noiseMap,t_i)[dbg_index],t_j)[dbg_index]<0.5){
									err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<419>";
									t_localBiome=5;
								}else{
									err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<420>";
									if(dbg_array(dbg_array(this.m_noiseMap,t_i)[dbg_index],t_j)[dbg_index]<0.6){
										err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<421>";
										t_localBiome=6;
									}else{
										err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<422>";
										if(dbg_array(dbg_array(this.m_noiseMap,t_i)[dbg_index],t_j)[dbg_index]<1.0){
											err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<423>";
											t_localBiome=7;
										}
									}
								}
							}
						}
					}
				}
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<426>";
			if(t_i==0 || t_i==t_width-1 || t_j==0 || t_j==t_height-1){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<427>";
				t_localBiome=8;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<430>";
			dbg_array(dbg_array(this.m_biomes,t_i)[dbg_index],t_j)[dbg_index]=t_localBiome;
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_makeLake=function(t_centerX,t_centerY){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<185>";
	var t_lakeWidth=((bb_random_Rnd2(1.0,5.0))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<186>";
	var t_lakeHeight=((bb_random_Rnd2(1.0,5.0))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<187>";
	print("Make a lake at "+String(t_centerX)+", "+String(t_centerY)+", Size: "+String(t_lakeWidth)+"x"+String(t_lakeHeight));
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<188>";
	var t_tempX=t_centerX-t_lakeWidth;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<189>";
	var t_tempY=t_centerY-t_lakeHeight;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<191>";
	var t_xTarget=t_tempX+(t_lakeWidth*2+1);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<192>";
	var t_yTarget=t_tempY+(t_lakeHeight*2+1);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<194>";
	dbg_array(dbg_array(this.m_biomes,t_centerX)[dbg_index],t_centerY)[dbg_index]=1;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<196>";
	for(var t_i=t_tempX;t_i<t_xTarget;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<197>";
		for(var t_j=t_tempY;t_j<t_yTarget;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<198>";
			if(t_i>0 && t_i<this.m_mapWidth-1 && t_j>0 && t_j<this.m_mapHeight-1 && !(dbg_array(dbg_array(this.m_biomes,t_i)[dbg_index],t_j)[dbg_index]==0)){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<199>";
				dbg_array(dbg_array(this.m_biomes,t_i)[dbg_index],t_j)[dbg_index]=1;
			}
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_makeRiver=function(t_startX,t_startY){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<133>";
	var t_currentX=t_startX;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<134>";
	var t_currentY=t_startY;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<135>";
	var t_localMinX=t_startX;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<136>";
	var t_localMinY=t_startY;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<137>";
	var t_lowestElevation=dbg_array(dbg_array(this.m_noiseMap,t_currentX)[dbg_index],t_currentY)[dbg_index];
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<138>";
	var t_riverEnd=false;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<139>";
	var t_riverLength=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<140>";
	dbg_array(dbg_array(this.m_biomes,t_currentX)[dbg_index],t_currentY)[dbg_index]=1;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<142>";
	while(t_riverEnd==false && t_riverLength<100){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<144>";
		if(t_currentX>0 && dbg_array(dbg_array(this.m_noiseMap,t_currentX-1)[dbg_index],t_currentY)[dbg_index]<t_lowestElevation){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<145>";
			t_localMinX=t_currentX-1;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<146>";
			t_localMinY=t_currentY;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<147>";
			t_lowestElevation=dbg_array(dbg_array(this.m_noiseMap,t_localMinX)[dbg_index],t_localMinY)[dbg_index];
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<149>";
		if(t_currentX<this.m_mapWidth-2 && dbg_array(dbg_array(this.m_noiseMap,t_currentX+1)[dbg_index],t_currentY)[dbg_index]<t_lowestElevation){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<150>";
			t_localMinX=t_currentX+1;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<151>";
			t_localMinY=t_currentY;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<152>";
			t_lowestElevation=dbg_array(dbg_array(this.m_noiseMap,t_localMinX)[dbg_index],t_localMinY)[dbg_index];
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<154>";
		if(t_currentY>0 && dbg_array(dbg_array(this.m_noiseMap,t_currentX)[dbg_index],t_currentY-1)[dbg_index]<t_lowestElevation){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<155>";
			t_localMinX=t_currentX;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<156>";
			t_localMinY=t_currentY-1;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<157>";
			t_lowestElevation=dbg_array(dbg_array(this.m_noiseMap,t_localMinX)[dbg_index],t_localMinY)[dbg_index];
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<159>";
		if(t_currentY<this.m_mapHeight-2 && dbg_array(dbg_array(this.m_noiseMap,t_currentX)[dbg_index],t_currentY+1)[dbg_index]<t_lowestElevation){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<160>";
			t_localMinX=t_currentX;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<161>";
			t_localMinY=t_currentY+1;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<162>";
			t_lowestElevation=dbg_array(dbg_array(this.m_noiseMap,t_localMinX)[dbg_index],t_localMinY)[dbg_index];
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<165>";
		if(dbg_array(dbg_array(this.m_biomes,t_localMinX)[dbg_index],t_localMinY)[dbg_index]==0){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<167>";
			t_riverEnd=true;
		}else{
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<168>";
			if(t_localMinX==t_currentX && t_localMinY==t_currentY){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<169>";
				this.p_makeLake(t_currentX,t_currentY);
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<170>";
				t_riverEnd=true;
			}else{
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<172>";
				dbg_array(dbg_array(this.m_biomes,t_localMinX)[dbg_index],t_localMinY)[dbg_index]=1;
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<173>";
				t_currentX=t_localMinX;
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<174>";
				t_currentY=t_localMinY;
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<175>";
				t_riverLength+=1;
			}
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_makeRivers=function(t_total){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<114>";
	var t_rivers=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<115>";
	var t_randX=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<116>";
	var t_randY=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<117>";
	var t_randBiome=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<118>";
	while(t_rivers<t_total){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<119>";
		t_randX=((bb_random_Rnd2(0.0,(this.m_mapWidth)))|0);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<120>";
		t_randY=((bb_random_Rnd2(0.0,(this.m_mapHeight)))|0);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<121>";
		t_randBiome=dbg_array(dbg_array(this.m_biomes,t_randX)[dbg_index],t_randY)[dbg_index];
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<122>";
		if(t_randBiome==5){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<123>";
			this.p_makeRiver(t_randX,t_randY);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<124>";
			print("River at "+String(t_randX)+", "+String(t_randY));
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<125>";
			t_rivers+=1;
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_selectRandomStartPoint=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<267>";
	var t_x=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<268>";
	var t_y=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<269>";
	var t_startPointSet=false;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<271>";
	while(!t_startPointSet){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<272>";
		t_x=((bb_random_Rnd2(200.0,400.0))|0);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<273>";
		t_y=((bb_random_Rnd2(130.0,330.0))|0);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<274>";
		if(dbg_array(dbg_array(dbg_object(this).m_biomes,t_x)[dbg_index],t_y)[dbg_index]==3 || dbg_array(dbg_array(dbg_object(this).m_biomes,t_x)[dbg_index],t_y)[dbg_index]==4){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<275>";
			dbg_object(this).m_playfieldN.p_CameraX2(t_x);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<276>";
			dbg_object(this).m_playfieldN.p_CameraY2(t_y);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<277>";
			t_startPointSet=true;
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_makeCaves=function(t_total){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<209>";
	var t_lowX=((dbg_object(this).m_playfieldN.p_CameraX()-50.0)|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<210>";
	var t_highX=((dbg_object(this).m_playfieldN.p_CameraX()+100.0)|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<211>";
	var t_lowY=((dbg_object(this).m_playfieldN.p_CameraY()-50.0)|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<212>";
	var t_highY=((dbg_object(this).m_playfieldN.p_CameraY()+100.0)|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<214>";
	var t_randX=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<215>";
	var t_randY=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<217>";
	var t_cavesMade=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<218>";
	var t_isReachable=false;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<220>";
	while(t_cavesMade<t_total){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<221>";
		t_isReachable=false;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<222>";
		t_randX=((bb_random_Rnd2((t_lowX),(t_highX)))|0);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<223>";
		t_randY=((bb_random_Rnd2((t_lowY),(t_highY)))|0);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<226>";
		if(dbg_array(dbg_array(this.m_biomes,t_randX)[dbg_index],t_randY)[dbg_index]==5){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<229>";
			if(t_randX>0 && dbg_array(dbg_array(this.m_biomes,t_randX-1)[dbg_index],t_randY)[dbg_index]==4){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<230>";
				t_isReachable=true;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<232>";
			if(t_randX<this.m_mapWidth-2 && dbg_array(dbg_array(this.m_biomes,t_randX+1)[dbg_index],t_randY)[dbg_index]==4){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<233>";
				t_isReachable=true;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<235>";
			if(t_randX>0 && t_randY>0 && dbg_array(dbg_array(this.m_biomes,t_randX-1)[dbg_index],t_randY-1)[dbg_index]==4){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<236>";
				t_isReachable=true;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<238>";
			if(t_randX<this.m_mapWidth-2 && t_randY>0 && dbg_array(dbg_array(this.m_biomes,t_randX+1)[dbg_index],t_randY-1)[dbg_index]==4){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<239>";
				t_isReachable=true;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<241>";
			if(t_randY>0 && dbg_array(dbg_array(this.m_biomes,t_randX)[dbg_index],t_randY-1)[dbg_index]==4){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<242>";
				t_isReachable=true;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<244>";
			if(t_randY>this.m_mapHeight-2 && dbg_array(dbg_array(this.m_biomes,t_randX)[dbg_index],t_randY+1)[dbg_index]==4){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<245>";
				t_isReachable=true;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<247>";
			if(t_randX>0 && t_randY<this.m_mapHeight-2 && dbg_array(dbg_array(this.m_biomes,t_randX-1)[dbg_index],t_randY+1)[dbg_index]==4){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<248>";
				t_isReachable=true;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<250>";
			if(t_randX<this.m_mapWidth-2 && t_randY<this.m_mapHeight-2 && dbg_array(dbg_array(this.m_biomes,t_randX+1)[dbg_index],t_randY+1)[dbg_index]==4){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<251>";
				t_isReachable=true;
			}
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<255>";
		if(t_isReachable){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<256>";
			print("Cave entrance at "+String(t_randX)+", "+String(t_randY));
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<257>";
			dbg_array(dbg_array(this.m_biomes,t_randX)[dbg_index],t_randY)[dbg_index]=8;
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<258>";
			dbg_array(this.m_caves,t_cavesMade)[dbg_index]=c_Level.m_new.call(new c_Level,t_randX,t_randY,150,100,"Cellular");
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<259>";
			t_cavesMade+=1;
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_determineEnemyType=function(t_enemies,t_habitat){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<440>";
	for(var t_i=0;t_i<t_enemies.length;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<441>";
		for(var t_j=0;t_j<dbg_array(t_enemies,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<442>";
			if(!(dbg_array(dbg_array(t_enemies,t_i)[dbg_index],t_j)[dbg_index]==-1)){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<443>";
				dbg_array(dbg_array(t_enemies,t_i)[dbg_index],t_j)[dbg_index]=dbg_array(dbg_array(t_habitat,t_i)[dbg_index],t_j)[dbg_index];
			}
		}
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_placeEnemies=function(t_width,t_height){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<452>";
	dbg_object(this).m_enemyPlacement=bb_noisetestscene_setArray(t_width,t_height);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<453>";
	bb_noisetestscene_randomlyAssignCells(dbg_object(this).m_enemyPlacement,1);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<454>";
	this.p_determineEnemyType(dbg_object(this).m_enemyPlacement,dbg_object(this).m_biomes);
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_OnStart=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<89>";
	print("Starting Noise Test");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<90>";
	var t_n=c_SimplexNoise.m_new.call(new c_SimplexNoise);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<91>";
	this.m_noiseMap=t_n.p_generateOctavedNoiseMap(this.m_mapWidth,this.m_mapHeight,5,0.5,1.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<92>";
	this.m_moisture=t_n.p_generateNoiseMap(this.m_mapWidth,this.m_mapHeight);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<93>";
	this.m_biomes=bb_noisetestscene_setArray(this.m_mapWidth,this.m_mapHeight);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<94>";
	this.p_determineBiomes(this.m_noiseMap,this.m_moisture,this.m_mapWidth,this.m_mapHeight);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<95>";
	this.p_makeRivers(25);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<96>";
	this.p_selectRandomStartPoint();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<97>";
	this.p_makeCaves(1);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<98>";
	this.p_placeEnemies(this.m_mapWidth,this.m_mapHeight);
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_OnStop=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<285>";
	print("Stopping Noise Test");
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_checkCameraBounds=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<339>";
	if(dbg_object(this).m_playfieldN.p_CameraX()<0.0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<340>";
		dbg_object(this).m_playfieldN.p_CameraX2(0.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<342>";
	if(dbg_object(this).m_playfieldN.p_CameraY()<0.0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<343>";
		dbg_object(this).m_playfieldN.p_CameraY2(0.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<350>";
	if(dbg_object(this).m_playfieldN.p_CameraX()+dbg_object(this).m_playfieldN.p_Width()/20.0>dbg_object(this).m_playfieldN.p_Width()){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<351>";
		dbg_object(this).m_playfieldN.p_CameraX2(dbg_object(this).m_playfieldN.p_Width()-dbg_object(this).m_playfieldN.p_Width()/20.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<353>";
	if(dbg_object(this).m_playfieldN.p_CameraY()+dbg_object(this).m_playfieldN.p_Height()/20.0>dbg_object(this).m_playfieldN.p_Height()){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<354>";
		dbg_object(this).m_playfieldN.p_CameraY2(dbg_object(this).m_playfieldN.p_Height()-dbg_object(this).m_playfieldN.p_Height()/20.0);
	}
	pop_err();
	return 0;
}
c_NoiseTestScene.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<291>";
	if((bb_input_KeyDown(17))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<293>";
		if((bb_input_KeyDown(65))!=0){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<294>";
			dbg_object(this).m_playfieldN.p_AlphaFade2(dbg_object(this).m_playfieldN.p_AlphaFade()-0.01);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<295>";
			if(dbg_object(this).m_playfieldN.p_AlphaFade()<0.0){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<295>";
				dbg_object(this).m_playfieldN.p_AlphaFade2(dbg_object(this).m_playfieldN.p_AlphaFade()+1.0);
			}
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<298>";
		if((bb_input_KeyDown(90))!=0){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<299>";
			dbg_object(this).m_playfieldN.p_ZoomX2(dbg_object(this).m_playfieldN.p_ZoomX()-0.01);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<300>";
			dbg_object(this).m_playfieldN.p_ZoomY2(dbg_object(this).m_playfieldN.p_ZoomY()-0.01);
		}
	}else{
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<305>";
		if((bb_input_KeyDown(65))!=0){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<306>";
			dbg_object(this).m_playfieldN.p_AlphaFade2(dbg_object(this).m_playfieldN.p_AlphaFade()+0.01);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<307>";
			if(dbg_object(this).m_playfieldN.p_AlphaFade()>1.0){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<307>";
				dbg_object(this).m_playfieldN.p_AlphaFade2(dbg_object(this).m_playfieldN.p_AlphaFade()-1.0);
			}
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<310>";
		if((bb_input_KeyDown(90))!=0){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<311>";
			dbg_object(this).m_playfieldN.p_ZoomX2(dbg_object(this).m_playfieldN.p_ZoomX()+0.01);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<312>";
			dbg_object(this).m_playfieldN.p_ZoomY2(dbg_object(this).m_playfieldN.p_ZoomY()+0.01);
		}
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<316>";
	if((bb_input_KeyDown(37))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<317>";
		dbg_object(this).m_playfieldN.p_CameraX2(dbg_object(this).m_playfieldN.p_CameraX()-1.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<320>";
	if((bb_input_KeyDown(39))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<321>";
		dbg_object(this).m_playfieldN.p_CameraX2(dbg_object(this).m_playfieldN.p_CameraX()+1.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<324>";
	if((bb_input_KeyDown(38))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<325>";
		dbg_object(this).m_playfieldN.p_CameraY2(dbg_object(this).m_playfieldN.p_CameraY()-1.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<328>";
	if((bb_input_KeyDown(40))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<329>";
		dbg_object(this).m_playfieldN.p_CameraY2(dbg_object(this).m_playfieldN.p_CameraY()+1.0);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<331>";
	this.p_checkCameraBounds();
	pop_err();
	return 0;
}
var bb_main_noiseTest=null;
var bb_app__updateRate=0;
function bb_app_UpdateRate(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<229>";
	pop_err();
	return bb_app__updateRate;
}
function bb_app_SetUpdateRate(t_hertz){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<224>";
	bb_app__updateRate=t_hertz;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/app.monkey<225>";
	bb_app__game.SetUpdateRate(t_hertz);
	pop_err();
}
function bb_app2_iStart(t_hertz){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<213>";
	if(t_hertz!=bb_app_UpdateRate()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<213>";
		bb_app_SetUpdateRate(t_hertz);
	}
	pop_err();
}
function bb_app2_iStart2(t_scene){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<220>";
	bb_app2_iNextScene=t_scene;
	pop_err();
}
function bb_app2_iStart3(t_scene,t_hertz){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<228>";
	bb_app2_iNextScene=t_scene;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<230>";
	if(t_hertz!=bb_app_UpdateRate()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/app.monkey<230>";
		bb_app_SetUpdateRate(t_hertz);
	}
	pop_err();
}
var bb_globals_iEnginePointer=null;
function bb_functions_iTimeCode(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<434>";
	pop_err();
	return dbg_object(bb_globals_iEnginePointer).m__timeCode;
}
function bb_graphics_GetScissor(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<297>";
	var t_=[dbg_object(bb_graphics_context).m_scissor_x,dbg_object(bb_graphics_context).m_scissor_y,dbg_object(bb_graphics_context).m_scissor_width,dbg_object(bb_graphics_context).m_scissor_height];
	pop_err();
	return t_;
}
function bb_graphics_GetScissor2(t_scissor){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<301>";
	dbg_array(t_scissor,0)[dbg_index]=dbg_object(bb_graphics_context).m_scissor_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<302>";
	dbg_array(t_scissor,1)[dbg_index]=dbg_object(bb_graphics_context).m_scissor_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<303>";
	dbg_array(t_scissor,2)[dbg_index]=dbg_object(bb_graphics_context).m_scissor_width;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<304>";
	dbg_array(t_scissor,3)[dbg_index]=dbg_object(bb_graphics_context).m_scissor_height;
	pop_err();
	return 0;
}
function bb_graphics_DebugRenderDevice(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<53>";
	if(!((bb_graphics_renderDevice)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<53>";
		error("Rendering operations can only be performed inside OnRender");
	}
	pop_err();
	return 0;
}
function bb_graphics_Cls(t_r,t_g,t_b){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<378>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<380>";
	bb_graphics_renderDevice.Cls(t_r,t_g,t_b);
	pop_err();
	return 0;
}
function bb_gfx_iCls(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<19>";
	var t_s=bb_graphics_GetScissor();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<21>";
	bb_graphics_SetScissor(0.0,0.0,(bb_app_DeviceWidth()),(bb_app_DeviceHeight()));
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<22>";
	bb_graphics_Cls(t_red,t_green,t_blue);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<24>";
	bb_graphics_SetScissor(dbg_array(t_s,0)[dbg_index],dbg_array(t_s,1)[dbg_index],dbg_array(t_s,2)[dbg_index],dbg_array(t_s,3)[dbg_index]);
	pop_err();
}
function c_iEngineObject(){
	c_iObject.call(this);
	this.m__autoDestroy=false;
}
c_iEngineObject.prototype=extend_class(c_iObject);
c_iEngineObject.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<10>";
	c_iObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<10>";
	pop_err();
	return this;
}
c_iEngineObject.prototype.p_OnRemove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<72>";
	pop_err();
	return 0;
}
c_iEngineObject.prototype.p_Remove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<79>";
	this.p_OnRemove();
	pop_err();
}
c_iEngineObject.m_destroyList=null;
c_iEngineObject.prototype.p_OnDestroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<65>";
	pop_err();
	return 0;
}
c_iEngineObject.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<38>";
	this.p_OnDestroy();
	pop_err();
}
c_iEngineObject.m_DestroyList2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<46>";
	if(c_iEngineObject.m_destroyList.p_Length()==0){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<48>";
	var t_o=c_iEngineObject.m_destroyList.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<49>";
	while((t_o)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<51>";
		c_iEngineObject.m_destroyList.p_Remove6(t_o);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<53>";
		if(dbg_object(t_o).m__inPool==0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<53>";
			t_o.p_Destroy();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/engineobject.monkey<55>";
		t_o=c_iEngineObject.m_destroyList.p_Ascend();
	}
	pop_err();
}
function c_iPlayfield(){
	c_iEngineObject.call(this);
	this.m__alphaFade=1.0;
	this.m__colorFade=1.0;
	this.m__visible=true;
	this.m__x=.0;
	this.m__y=.0;
	this.m__vwidth=0;
	this.m__scaleX=1.0;
	this.m__vheight=0;
	this.m__scaleY=1.0;
	this.m__autoCls=true;
	this.m__clsRed=.0;
	this.m__clsGreen=.0;
	this.m__clsBlue=.0;
	this.m__zoomPointX=.0;
	this.m__zoomPointY=.0;
	this.m__zoomX=1.0;
	this.m__zoomY=1.0;
	this.m__rotation=.0;
	this.m__backgroundRender=false;
	this.m__cameraX=.0;
	this.m__cameraY=.0;
	this.m__layerList=c_iList2.m_new.call(new c_iList2);
	this.m__currentLayer=null;
	this.m__height=0;
	this.m__width=0;
	this.m__guiEnabled=true;
	this.m__guiList=new_object_array(1);
	this.m__guiPage=0;
	this.m__guiLastObject=null;
	this.m__nextGuiPage=-1;
	this.m__z=.0;
}
c_iPlayfield.prototype=extend_class(c_iEngineObject);
c_iPlayfield.prototype.p_SetAlpha=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1255>";
	bb_graphics_SetAlpha(t_alpha*dbg_object(bb_globals_iEnginePointer).m__alphaFade*this.m__alphaFade);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1257>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_SetColor=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1267>";
	bb_graphics_SetColor(t_red*dbg_object(bb_globals_iEnginePointer).m__colorFade*this.m__colorFade,t_green*dbg_object(bb_globals_iEnginePointer).m__colorFade*this.m__colorFade,t_blue*dbg_object(bb_globals_iEnginePointer).m__colorFade*this.m__colorFade);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1269>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_OnBackgroundRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<728>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<748>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_PositionX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<784>";
	pop_err();
	return this.m__x;
}
c_iPlayfield.prototype.p_PositionX2=function(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<791>";
	this.m__x=t_x;
	pop_err();
}
c_iPlayfield.prototype.p_CameraX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<278>";
	pop_err();
	return this.m__cameraX;
}
c_iPlayfield.prototype.p_CameraX2=function(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<285>";
	this.m__cameraX=t_x;
	pop_err();
}
c_iPlayfield.prototype.p_PositionY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<798>";
	pop_err();
	return this.m__y;
}
c_iPlayfield.prototype.p_PositionY2=function(t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<805>";
	this.m__y=t_y;
	pop_err();
}
c_iPlayfield.prototype.p_CameraY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<292>";
	pop_err();
	return this.m__cameraY;
}
c_iPlayfield.prototype.p_CameraY2=function(t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<299>";
	this.m__cameraY=t_y;
	pop_err();
}
c_iPlayfield.prototype.p_VWidth=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1765>";
	var t_=(this.m__vwidth);
	pop_err();
	return t_;
}
c_iPlayfield.prototype.p_ZoomPointX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1889>";
	pop_err();
	return this.m__zoomPointX;
}
c_iPlayfield.prototype.p_ZoomPointX2=function(t_zoomPointX){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1931>";
	this.m__zoomPointX=t_zoomPointX;
	pop_err();
}
c_iPlayfield.prototype.p_VHeight=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1681>";
	var t_=(this.m__vheight);
	pop_err();
	return t_;
}
c_iPlayfield.prototype.p_ZoomPointY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1896>";
	pop_err();
	return this.m__zoomPointY;
}
c_iPlayfield.prototype.p_ZoomPointY2=function(t_zoomPointY){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1938>";
	this.m__zoomPointY=t_zoomPointY;
	pop_err();
}
c_iPlayfield.prototype.p_ZoomPoint=function(t_zoomPointX,t_zoomPointY){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1880>";
	this.p_ZoomPointX2(t_zoomPointX);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1881>";
	this.p_ZoomPointY2(t_zoomPointY);
	pop_err();
}
c_iPlayfield.prototype.p_VHeight2=function(t_height){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1695>";
	var t_h=(bb_app_DeviceHeight());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1697>";
	var t_s=t_h/(t_height);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1699>";
	this.m__vwidth=(((this.m__width)/t_s)|0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1701>";
	this.m__vheight=t_height;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1703>";
	this.m__scaleX=t_s;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1704>";
	this.m__scaleY=t_s;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1706>";
	this.p_ZoomPoint2();
	pop_err();
}
c_iPlayfield.prototype.p_ZoomPoint2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1870>";
	this.p_ZoomPointX2(this.p_VWidth()*.5);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1871>";
	this.p_ZoomPointY2(this.p_VHeight()*.5);
	pop_err();
}
c_iPlayfield.prototype.p_VWidth2=function(t_width){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1779>";
	var t_w=(bb_app_DeviceWidth());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1781>";
	var t_s=t_w/(t_width);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1783>";
	this.m__vwidth=t_width;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1785>";
	this.m__vheight=(((this.m__height)/t_s)|0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1787>";
	this.m__scaleX=t_s;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1788>";
	this.m__scaleY=t_s;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1790>";
	this.p_ZoomPoint2();
	pop_err();
}
c_iPlayfield.prototype.p_OnTopRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<752>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_RenderGui=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1023>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1025>";
	if(dbg_object(bb_globals_iEnginePointer).m__borders){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1025>";
		bb_gfx_iSetScissor(this.m__x,this.m__y,(this.m__vwidth)*this.m__scaleY,(this.m__vheight)*this.m__scaleY);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1028>";
	bb_graphics_Scale(this.m__scaleX,this.m__scaleY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1030>";
	bb_graphics_Translate(this.m__x/this.m__scaleX,this.m__y/this.m__scaleY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1032>";
	var t_g=dbg_array(this.m__guiList,this.m__guiPage)[dbg_index].p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1033>";
	while((t_g)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1035>";
		if(dbg_object(t_g).m__enabled==1 && dbg_object(t_g).m__visible){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1036>";
			if(dbg_object(t_g).m__ghost){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1036>";
				t_g.p_RenderGhost();
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1036>";
				t_g.p_Render();
			}
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1039>";
		t_g=dbg_array(this.m__guiList,this.m__guiPage)[dbg_index].p_Ascend();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1043>";
	if(((this.m__guiLastObject)!=null) && dbg_object(this.m__guiLastObject).m__enabled==1 && dbg_object(this.m__guiLastObject).m__visible){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1045>";
		bb_graphics_SetAlpha(.75);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1046>";
		bb_graphics_SetBlend(0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1047>";
		bb_graphics_SetColor(1.0,1.0,1.0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1048>";
		bb_graphics_Rotate(0.0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1049>";
		bb_graphics_DrawRect(0.0,0.0,(this.m__vwidth),(this.m__vheight));
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1051>";
		if(dbg_object(this.m__guiLastObject).m__ghost){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1051>";
			this.m__guiLastObject.p_RenderGhost();
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1051>";
			this.m__guiLastObject.p_Render();
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1055>";
	c_iStack2D.m_Pop();
	pop_err();
}
c_iPlayfield.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<933>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<935>";
	if(dbg_object(bb_globals_iEnginePointer).m__borders){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<935>";
		bb_gfx_iSetScissor(this.m__x,this.m__y,(this.m__vwidth)*this.m__scaleX,(this.m__vheight)*this.m__scaleY);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<938>";
	if(this.m__autoCls){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<939>";
		bb_graphics_Cls(this.m__clsRed*dbg_object(bb_globals_iEnginePointer).m__colorFade*this.m__colorFade,this.m__clsGreen*dbg_object(bb_globals_iEnginePointer).m__colorFade*this.m__colorFade,this.m__clsBlue*dbg_object(bb_globals_iEnginePointer).m__colorFade*this.m__colorFade);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<942>";
	bb_graphics_Scale(this.m__scaleX,this.m__scaleY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<944>";
	bb_graphics_Translate(this.m__x/this.m__scaleX+this.m__zoomPointX,this.m__y/this.m__scaleY+this.m__zoomPointY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<945>";
	bb_graphics_Scale(this.m__zoomX,this.m__zoomY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<946>";
	bb_graphics_Translate(-(this.m__x/this.m__scaleX+this.m__zoomPointX),-(this.m__y/this.m__scaleY+this.m__zoomPointY));
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<948>";
	bb_graphics_Translate(this.m__x/this.m__scaleX+this.m__zoomPointX,this.m__y/this.m__scaleY+this.m__zoomPointY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<949>";
	bb_graphics_Rotate(this.m__rotation*(bb_globals_iRotation));
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<950>";
	bb_graphics_Translate(-(this.m__x/this.m__scaleX+this.m__zoomPointX),-(this.m__y/this.m__scaleY+this.m__zoomPointY));
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<952>";
	if(this.m__backgroundRender){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<953>";
		c_iStack2D.m_Push();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<954>";
		this.p_OnBackgroundRender();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<955>";
		c_iStack2D.m_Pop();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<958>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<960>";
	bb_graphics_Translate(this.m__x/this.m__scaleX-this.m__cameraX,this.m__y/this.m__scaleY-this.m__cameraY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<962>";
	if(dbg_object(bb_globals_iEnginePointer).m__renderToPlayfield==true){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<964>";
		c_iStack2D.m_Push();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<966>";
		bb_globals_iEnginePointer.p_SetAlpha(1.0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<967>";
		bb_globals_iEnginePointer.p_SetColor(255.0,255.0,255.0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<969>";
		bb_globals_iEnginePointer.p_OnRender();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<971>";
		c_iStack2D.m_Pop();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<975>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<976>";
	this.p_OnRender();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<977>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<979>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<981>";
	this.m__currentLayer=this.m__layerList.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<983>";
	while((this.m__currentLayer)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<985>";
		if(((dbg_object(this.m__currentLayer).m__enabled)!=0) && dbg_object(this.m__currentLayer).m__visible){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<985>";
			this.m__currentLayer.p_Render();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<987>";
		this.m__currentLayer=this.m__layerList.p_Ascend();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<991>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<993>";
	bb_graphics_Translate(this.m__x/this.m__scaleX-this.m__cameraX,this.m__y/this.m__scaleY-this.m__cameraY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<995>";
	if(dbg_object(bb_globals_iEnginePointer).m__renderToPlayfield==true){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<997>";
		c_iStack2D.m_Push();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<999>";
		bb_globals_iEnginePointer.p_SetAlpha(1.0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1000>";
		bb_globals_iEnginePointer.p_SetColor(255.0,255.0,255.0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1002>";
		bb_globals_iEnginePointer.p_OnTopRender();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1004>";
		c_iStack2D.m_Pop();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1008>";
	this.p_OnTopRender();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1010>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1012>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1014>";
	if(this.m__guiEnabled){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1014>";
		this.p_RenderGui();
	}
	pop_err();
}
c_iPlayfield.prototype.p_GuiPage=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<478>";
	if(this.m__nextGuiPage!=-1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<478>";
		pop_err();
		return this.m__nextGuiPage;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<478>";
		pop_err();
		return this.m__guiPage;
	}
}
c_iPlayfield.prototype.p_InitGuiPages=function(t_pages){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<537>";
	if(this.m__guiList.length<=t_pages){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<539>";
		this.m__guiList=resize_object_array(this.m__guiList,t_pages+1);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<541>";
		for(var t_p=0;t_p<this.m__guiList.length;t_p=t_p+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<542>";
			if(!((dbg_array(this.m__guiList,t_p)[dbg_index])!=null)){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<542>";
				dbg_array(this.m__guiList,t_p)[dbg_index]=c_iList4.m_new.call(new c_iList4);
			}
		}
	}
	pop_err();
}
c_iPlayfield.prototype.p_OnGuiPageChange2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<744>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_GuiPage2=function(t_page){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<486>";
	this.p_InitGuiPages(t_page);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<488>";
	if(this.m__guiPage!=t_page){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<489>";
		this.p_OnGuiPageChange2();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<490>";
		bb_globals_iEnginePointer.p_OnGuiPageChange(this);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<493>";
	this.m__nextGuiPage=t_page;
	pop_err();
}
c_iPlayfield.prototype.p_Set2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1247>";
	if((bb_globals_iEnginePointer)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1247>";
		dbg_object(bb_globals_iEnginePointer).m__playfieldPointer=this;
	}
	pop_err();
}
c_iPlayfield.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<737>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_SystemInit=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1504>";
	this.p_Set2();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1506>";
	this.m__height=bb_app_DeviceHeight();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1507>";
	this.m__width=bb_app_DeviceWidth();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1509>";
	this.m__vheight=bb_app_DeviceHeight();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1510>";
	this.m__vwidth=bb_app_DeviceWidth();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1512>";
	dbg_array(this.m__guiList,0)[dbg_index]=c_iList4.m_new.call(new c_iList4);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1514>";
	this.p_ZoomPoint2();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1516>";
	this.p_OnCreate();
	pop_err();
}
c_iPlayfield.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<693>";
	c_iEngineObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<694>";
	this.p_SystemInit();
	pop_err();
	return this;
}
c_iPlayfield.m_new2=function(t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<700>";
	c_iEngineObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<702>";
	this.p_SystemInit();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<704>";
	this.m__vwidth=t_width;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<705>";
	this.m__vheight=t_height;
	pop_err();
	return this;
}
c_iPlayfield.m_new3=function(t_x,t_y,t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<712>";
	c_iEngineObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<714>";
	this.p_SystemInit();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<716>";
	this.m__x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<717>";
	this.m__y=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<719>";
	this.m__vwidth=t_width;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<720>";
	this.m__vheight=t_height;
	pop_err();
	return this;
}
c_iPlayfield.prototype.p_UpdateGui=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1586>";
	if(this.m__nextGuiPage>=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1587>";
		this.m__guiPage=this.m__nextGuiPage;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1588>";
		this.m__nextGuiPage=-1;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1591>";
	if((this.m__guiLastObject)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1593>";
		this.m__guiLastObject.p_UpdateWorldXY();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1595>";
		if(dbg_object(this.m__guiLastObject).m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1597>";
			if(dbg_object(this.m__guiLastObject).m__ghost){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1599>";
				this.m__guiLastObject.p_UpdateGhost();
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1603>";
				this.m__guiLastObject.p_UpdateInput();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1604>";
				this.m__guiLastObject.p_Update();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1606>";
				if((this.m__guiLastObject)!=null){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1606>";
					this.m__guiLastObject.p_OnUpdate();
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1614>";
		var t_g=null;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1616>";
		t_g=dbg_array(this.m__guiList,this.m__guiPage)[dbg_index].p_Last();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1618>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1620>";
			t_g.p_UpdateWorldXY();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1622>";
			if(dbg_object(t_g).m__enabled==1 && !dbg_object(t_g).m__ghost){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1622>";
				t_g.p_UpdateInput();
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1624>";
			t_g=dbg_array(this.m__guiList,this.m__guiPage)[dbg_index].p_Descend();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1628>";
		t_g=dbg_array(this.m__guiList,this.m__guiPage)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1629>";
		while((t_g)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1631>";
			if(dbg_object(t_g).m__enabled==1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1633>";
				if(dbg_object(t_g).m__ghost){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1635>";
					t_g.p_UpdateGhost();
				}else{
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1639>";
					t_g.p_Update();
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1640>";
					t_g.p_OnUpdate();
				}
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1645>";
				if(dbg_object(t_g).m__enabled>1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1645>";
					dbg_object(t_g).m__enabled=dbg_object(t_g).m__enabled-1;
				}
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1648>";
			t_g=dbg_array(this.m__guiList,this.m__guiPage)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iPlayfield.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<756>";
	pop_err();
	return 0;
}
c_iPlayfield.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1576>";
	this.p_UpdateGui();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1577>";
	this.p_OnUpdate();
	pop_err();
}
c_iPlayfield.prototype.p_AttachLast=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<244>";
	dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_AddLast(this);
	pop_err();
}
c_iPlayfield.prototype.p_AttachLast2=function(t_engine){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<251>";
	dbg_object(t_engine).m__playfieldList.p_AddLast(this);
	pop_err();
}
c_iPlayfield.prototype.p_AutoCls=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<259>";
	this.m__clsRed=(t_red);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<260>";
	this.m__clsGreen=(t_green);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<261>";
	this.m__clsBlue=(t_blue);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<263>";
	this.m__autoCls=true;
	pop_err();
}
c_iPlayfield.prototype.p_AutoCls2=function(t_state){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<271>";
	this.m__autoCls=t_state;
	pop_err();
}
c_iPlayfield.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1798>";
	var t_=(this.m__width);
	pop_err();
	return t_;
}
c_iPlayfield.prototype.p_Width2=function(t_width){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1805>";
	this.m__width=((t_width)|0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1806>";
	this.m__vwidth=((t_width)|0);
	pop_err();
}
c_iPlayfield.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<521>";
	var t_=(this.m__height);
	pop_err();
	return t_;
}
c_iPlayfield.prototype.p_Height2=function(t_height){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<528>";
	this.m__height=((t_height)|0);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<529>";
	this.m__vheight=((t_height)|0);
	pop_err();
}
c_iPlayfield.prototype.p_Position=function(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<764>";
	this.m__x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<765>";
	this.m__y=t_y;
	pop_err();
}
c_iPlayfield.prototype.p_Position2=function(t_x,t_y,t_z){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<774>";
	this.m__x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<775>";
	this.m__y=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<776>";
	this.m__z=t_z;
	pop_err();
}
c_iPlayfield.prototype.p_AlphaFade=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<216>";
	pop_err();
	return this.m__alphaFade;
}
c_iPlayfield.prototype.p_AlphaFade2=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<223>";
	this.m__alphaFade=t_alpha;
	pop_err();
}
c_iPlayfield.prototype.p_ZoomX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1903>";
	pop_err();
	return this.m__zoomX;
}
c_iPlayfield.prototype.p_ZoomX2=function(t_zoomX){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1917>";
	this.m__zoomX=t_zoomX;
	pop_err();
}
c_iPlayfield.prototype.p_ZoomY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1910>";
	pop_err();
	return this.m__zoomY;
}
c_iPlayfield.prototype.p_ZoomY2=function(t_zoomY){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<1924>";
	this.m__zoomY=t_zoomY;
	pop_err();
}
c_iPlayfield.prototype.p_Remove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<837>";
	if(this.m__tattoo){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<839>";
		this.m__enabled=0;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<843>";
		this.m__enabled=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<845>";
		dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_Remove2(this);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<847>";
		if(this.m__autoDestroy){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/playfield.monkey<847>";
			c_iEngineObject.m_destroyList.p_AddLast4(this);
		}
	}
	pop_err();
}
function c_iList(){
	Object.call(this);
	this.m__length=0;
	this.m__index=0;
	this.m__data=[];
}
c_iList.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList.prototype.p_Length=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<170>";
	pop_err();
	return this.m__length;
}
c_iList.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>";
	this.m__index=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
	if((this.m__length)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
		pop_err();
		return dbg_array(this.m__data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>";
	this.m__index=this.m__index+1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
	if(this.m__index>=0 && this.m__index<this.m__length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
		pop_err();
		return dbg_array(this.m__data,this.m__index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList.prototype.p_Get=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<128>";
	pop_err();
	return dbg_array(this.m__data,t_index)[dbg_index];
}
c_iList.prototype.p_AddLast=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<55>";
	if(this.m__length==this.m__data.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<56>";
		this.m__data=resize_object_array(this.m__data,this.m__length*2+10);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<58>";
	dbg_array(this.m__data,this.m__length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<59>";
	this.m__length=this.m__length+1;
	pop_err();
}
c_iList.prototype.p_Position3=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>";
	for(var t_i=0;t_i<this.m__length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
		if(dbg_array(this.m__data,t_i)[dbg_index]==t_value){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
			pop_err();
			return t_i;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>";
	pop_err();
	return -1;
}
c_iList.prototype.p_RemoveFromIndex=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>";
	for(var t_i=t_index;t_i<this.m__length-1;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>";
		dbg_array(this.m__data,t_i)[dbg_index]=dbg_array(this.m__data,t_i+1)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>";
	this.m__length=this.m__length-1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>";
	this.m__index=this.m__index-1;
	pop_err();
}
c_iList.prototype.p_Remove2=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>";
	var t_p=this.p_Position3(t_value);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
	if(t_p>=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
		this.p_RemoveFromIndex(t_p);
	}
	pop_err();
}
function c_iStack2D(){
	Object.call(this);
	this.m__alpha=.0;
	this.m__blend=0;
	this.m__color=[];
	this.m__matrix=[];
	this.m__scissor=[];
}
c_iStack2D.m__length=0;
c_iStack2D.m__data=[];
c_iStack2D.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<10>";
	pop_err();
	return this;
}
c_iStack2D.m_Push=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<48>";
	if(c_iStack2D.m__length==c_iStack2D.m__data.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<50>";
		c_iStack2D.m__data=resize_object_array(c_iStack2D.m__data,c_iStack2D.m__length*2+10);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<52>";
		for(var t_i=0;t_i<c_iStack2D.m__data.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<53>";
			if(!((dbg_array(c_iStack2D.m__data,t_i)[dbg_index])!=null)){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<53>";
				dbg_array(c_iStack2D.m__data,t_i)[dbg_index]=c_iStack2D.m_new.call(new c_iStack2D);
			}
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<58>";
	var t_o=dbg_array(c_iStack2D.m__data,c_iStack2D.m__length)[dbg_index];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<60>";
	dbg_object(t_o).m__alpha=bb_graphics_GetAlpha();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<61>";
	dbg_object(t_o).m__blend=bb_graphics_GetBlend();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<62>";
	dbg_object(t_o).m__color=bb_graphics_GetColor();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<63>";
	dbg_object(t_o).m__matrix=bb_graphics_GetMatrix();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<64>";
	dbg_object(t_o).m__scissor=bb_graphics_GetScissor();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<66>";
	c_iStack2D.m__length=c_iStack2D.m__length+1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<68>";
	pop_err();
	return t_o;
}
c_iStack2D.m_Pop=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<34>";
	c_iStack2D.m__length=c_iStack2D.m__length-1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<36>";
	var t_o=dbg_array(c_iStack2D.m__data,c_iStack2D.m__length)[dbg_index];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<38>";
	bb_graphics_SetAlpha(dbg_object(t_o).m__alpha);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<39>";
	bb_graphics_SetBlend(dbg_object(t_o).m__blend);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<40>";
	bb_graphics_SetMatrix(dbg_array(dbg_object(t_o).m__matrix,0)[dbg_index],dbg_array(dbg_object(t_o).m__matrix,1)[dbg_index],dbg_array(dbg_object(t_o).m__matrix,2)[dbg_index],dbg_array(dbg_object(t_o).m__matrix,3)[dbg_index],dbg_array(dbg_object(t_o).m__matrix,4)[dbg_index],dbg_array(dbg_object(t_o).m__matrix,5)[dbg_index]);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<41>";
	bb_graphics_SetColor(dbg_array(dbg_object(t_o).m__color,0)[dbg_index],dbg_array(dbg_object(t_o).m__color,1)[dbg_index],dbg_array(dbg_object(t_o).m__color,2)[dbg_index]);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/stack2d.monkey<42>";
	bb_graphics_SetScissor(dbg_array(dbg_object(t_o).m__scissor,0)[dbg_index],dbg_array(dbg_object(t_o).m__scissor,1)[dbg_index],dbg_array(dbg_object(t_o).m__scissor,2)[dbg_index],dbg_array(dbg_object(t_o).m__scissor,3)[dbg_index]);
	pop_err();
}
function bb_graphics_GetAlpha(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<276>";
	pop_err();
	return dbg_object(bb_graphics_context).m_alpha;
}
function bb_graphics_GetBlend(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<285>";
	pop_err();
	return dbg_object(bb_graphics_context).m_blend;
}
function bb_graphics_GetColor(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<261>";
	var t_=[dbg_object(bb_graphics_context).m_color_r,dbg_object(bb_graphics_context).m_color_g,dbg_object(bb_graphics_context).m_color_b];
	pop_err();
	return t_;
}
function bb_graphics_GetColor2(t_color){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<265>";
	dbg_array(t_color,0)[dbg_index]=dbg_object(bb_graphics_context).m_color_r;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<266>";
	dbg_array(t_color,1)[dbg_index]=dbg_object(bb_graphics_context).m_color_g;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<267>";
	dbg_array(t_color,2)[dbg_index]=dbg_object(bb_graphics_context).m_color_b;
	pop_err();
	return 0;
}
function bb_graphics_GetMatrix(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<323>";
	var t_=[dbg_object(bb_graphics_context).m_ix,dbg_object(bb_graphics_context).m_iy,dbg_object(bb_graphics_context).m_jx,dbg_object(bb_graphics_context).m_jy,dbg_object(bb_graphics_context).m_tx,dbg_object(bb_graphics_context).m_ty];
	pop_err();
	return t_;
}
function bb_graphics_GetMatrix2(t_matrix){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<327>";
	dbg_array(t_matrix,0)[dbg_index]=dbg_object(bb_graphics_context).m_ix;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<327>";
	dbg_array(t_matrix,1)[dbg_index]=dbg_object(bb_graphics_context).m_iy;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<328>";
	dbg_array(t_matrix,2)[dbg_index]=dbg_object(bb_graphics_context).m_jx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<328>";
	dbg_array(t_matrix,3)[dbg_index]=dbg_object(bb_graphics_context).m_jy;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<329>";
	dbg_array(t_matrix,4)[dbg_index]=dbg_object(bb_graphics_context).m_tx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<329>";
	dbg_array(t_matrix,5)[dbg_index]=dbg_object(bb_graphics_context).m_ty;
	pop_err();
	return 0;
}
function bb_gfx_iSetScissor(t_x,t_y,t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<504>";
	if(t_x<0.0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<506>";
		if(-t_x>=t_width){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<507>";
			t_x=0.0;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<508>";
			t_width=0.0;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<510>";
			t_width=t_width+t_x;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<511>";
			t_x=0.0;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<516>";
	if(t_x+t_width>(bb_app_DeviceWidth())){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<518>";
		if(t_x>=(bb_app_DeviceWidth())){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<519>";
			t_x=(bb_app_DeviceWidth());
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<520>";
			t_width=0.0;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<522>";
			t_width=(bb_app_DeviceWidth()-((t_x)|0));
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<527>";
	if(t_y<0.0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<529>";
		if(-t_y>=t_height){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<530>";
			t_y=0.0;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<531>";
			t_height=0.0;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<533>";
			t_height=t_height+t_y;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<534>";
			t_y=0.0;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<539>";
	if(t_y+t_height>(bb_app_DeviceHeight())){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<541>";
		if(t_y>=(bb_app_DeviceHeight())){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<542>";
			t_y=(bb_app_DeviceHeight());
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<543>";
			t_height=0.0;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<545>";
			t_height=(bb_app_DeviceHeight()-((t_y)|0));
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<550>";
	bb_graphics_SetScissor(t_x,t_y,t_width,t_height);
	pop_err();
}
function bb_graphics_Transform(t_ix,t_iy,t_jx,t_jy,t_tx,t_ty){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<355>";
	var t_ix2=t_ix*dbg_object(bb_graphics_context).m_ix+t_iy*dbg_object(bb_graphics_context).m_jx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<356>";
	var t_iy2=t_ix*dbg_object(bb_graphics_context).m_iy+t_iy*dbg_object(bb_graphics_context).m_jy;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<357>";
	var t_jx2=t_jx*dbg_object(bb_graphics_context).m_ix+t_jy*dbg_object(bb_graphics_context).m_jx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<358>";
	var t_jy2=t_jx*dbg_object(bb_graphics_context).m_iy+t_jy*dbg_object(bb_graphics_context).m_jy;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<359>";
	var t_tx2=t_tx*dbg_object(bb_graphics_context).m_ix+t_ty*dbg_object(bb_graphics_context).m_jx+dbg_object(bb_graphics_context).m_tx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<360>";
	var t_ty2=t_tx*dbg_object(bb_graphics_context).m_iy+t_ty*dbg_object(bb_graphics_context).m_jy+dbg_object(bb_graphics_context).m_ty;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<361>";
	bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
	pop_err();
	return 0;
}
function bb_graphics_Transform2(t_m){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<351>";
	bb_graphics_Transform(dbg_array(t_m,0)[dbg_index],dbg_array(t_m,1)[dbg_index],dbg_array(t_m,2)[dbg_index],dbg_array(t_m,3)[dbg_index],dbg_array(t_m,4)[dbg_index],dbg_array(t_m,5)[dbg_index]);
	pop_err();
	return 0;
}
function bb_graphics_Scale(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<369>";
	bb_graphics_Transform(t_x,0.0,0.0,t_y,0.0,0.0);
	pop_err();
	return 0;
}
function bb_graphics_Translate(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<365>";
	bb_graphics_Transform(1.0,0.0,0.0,1.0,t_x,t_y);
	pop_err();
	return 0;
}
var bb_globals_iRotation=0;
function bb_graphics_Rotate(t_angle){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<373>";
	bb_graphics_Transform(Math.cos((t_angle)*D2R),-Math.sin((t_angle)*D2R),Math.sin((t_angle)*D2R),Math.cos((t_angle)*D2R),0.0,0.0);
	pop_err();
	return 0;
}
function c_iLayer(){
	c_iEngineObject.call(this);
	this.m__visible=true;
	this.m__stack2D=null;
	this.m__cameraSpeedX=1.0;
	this.m__cameraSpeedY=1.0;
	this.m__objectList=[];
	this.m__blockWidth=0;
	this.m__blockWidthExtra=0;
	this.m__blockHeight=0;
	this.m__blockHeightExtra=0;
	this.m__lastUpdate=0;
	this.m__loaderCache=null;
	this.m__alphaFade=1.0;
	this.m__colorFade=1.0;
}
c_iLayer.prototype=extend_class(c_iEngineObject);
c_iLayer.prototype.p_CameraSpeedX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<164>";
	pop_err();
	return this.m__cameraSpeedX;
}
c_iLayer.prototype.p_CameraSpeedX2=function(t_speedX){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<171>";
	this.m__cameraSpeedX=t_speedX;
	pop_err();
}
c_iLayer.prototype.p_CameraSpeedY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<178>";
	pop_err();
	return this.m__cameraSpeedY;
}
c_iLayer.prototype.p_CameraSpeedY2=function(t_speedY){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<185>";
	this.m__cameraSpeedY=t_speedY;
	pop_err();
}
c_iLayer.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<623>";
	pop_err();
	return 0;
}
c_iLayer.prototype.p_Grid=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<325>";
	if(this.m__objectList.length>1 || dbg_array(this.m__objectList,0)[dbg_index].length>1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<325>";
		pop_err();
		return true;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<325>";
		pop_err();
		return false;
	}
}
c_iLayer.prototype.p_GridWidth=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<360>";
	var t_=this.m__objectList.length;
	pop_err();
	return t_;
}
c_iLayer.prototype.p_GridHeight=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<332>";
	var t_=dbg_array(this.m__objectList,0)[dbg_index].length;
	pop_err();
	return t_;
}
c_iLayer.prototype.p_RenderLayerObjects=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<760>";
	if(this.p_Grid()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<762>";
		var t_startX=((dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__cameraX/(this.m__blockWidth)-(this.m__blockWidthExtra))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<763>";
		var t_startY=((dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__cameraY/(this.m__blockHeight)-(this.m__blockHeightExtra))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<765>";
		if(t_startX<0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<765>";
			t_startX=0;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<766>";
		if(t_startY<0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<766>";
			t_startY=0;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<768>";
		var t_stopX=(((t_startX)+dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_VWidth()/(this.m__blockWidth)+1.0+1.0+(this.m__blockWidthExtra*2))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<769>";
		var t_stopY=(((t_startY)+dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_VHeight()/(this.m__blockHeight)+1.0+1.0+(this.m__blockHeightExtra*2))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<771>";
		if(t_stopX>this.p_GridWidth()){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<771>";
			t_stopX=this.p_GridWidth();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<772>";
		if(t_stopY>this.p_GridHeight()){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<772>";
			t_stopY=this.p_GridHeight();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<774>";
		for(var t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<776>";
			for(var t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<778>";
				for(var t_i=0;t_i<dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index].p_Length();t_i=t_i+1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<779>";
					if(dbg_object(dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index].p_Get(t_i)).m__enabled==1 && dbg_object(dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index].p_Get(t_i)).m__visible){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<779>";
						dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index].p_Get(t_i).p_Render();
					}
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<788>";
		var t_o=dbg_array(dbg_array(this.m__objectList,0)[dbg_index],0)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<789>";
		while((t_o)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<791>";
			if(dbg_object(t_o).m__enabled==1 && dbg_object(t_o).m__visible){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<791>";
				t_o.p_Render();
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<793>";
			t_o=dbg_array(dbg_array(this.m__objectList,0)[dbg_index],0)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iLayer.prototype.p_OnTopRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<630>";
	pop_err();
	return 0;
}
c_iLayer.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<723>";
	this.m__stack2D=c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<725>";
	if(this.p_CameraSpeedX()!=0.0 || this.p_CameraSpeedY()!=0.0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<726>";
		bb_graphics_Translate(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_PositionX()/dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__scaleX-dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_CameraX()*dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__currentLayer.p_CameraSpeedX(),dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_PositionY()/dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__scaleY-dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_CameraY()*dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__currentLayer.p_CameraSpeedY());
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<729>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<730>";
	this.p_OnRender();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<731>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<733>";
	this.p_RenderLayerObjects();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<735>";
	c_iStack2D.m_Push();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<736>";
	this.p_OnTopRender();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<737>";
	c_iStack2D.m_Pop();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<739>";
	c_iStack2D.m_Pop();
	pop_err();
}
c_iLayer.prototype.p_UpdateWorldXY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1000>";
	if(this.p_Grid()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1002>";
		var t_startX=((dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__cameraX/(this.m__blockWidth)-(this.m__blockWidthExtra))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1003>";
		var t_startY=((dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__cameraY/(this.m__blockHeight)-(this.m__blockHeightExtra))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1005>";
		if(t_startX<0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1005>";
			t_startX=0;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1006>";
		if(t_startY<0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1006>";
			t_startY=0;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1008>";
		var t_stopX=(((t_startX)+dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_VWidth()/(this.m__blockWidth)+1.0+1.0+(this.m__blockWidthExtra*2))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1009>";
		var t_stopY=(((t_startY)+dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_VHeight()/(this.m__blockHeight)+1.0+1.0-(this.m__blockHeightExtra*2))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1011>";
		if(t_stopX>this.p_GridWidth()){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1011>";
			t_stopX=this.p_GridWidth();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1012>";
		if(t_stopY>this.p_GridHeight()){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1012>";
			t_stopY=this.p_GridHeight();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1014>";
		for(var t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1016>";
			for(var t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1018>";
				for(var t_i=0;t_i<dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index].p_Length();t_i=t_i+1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1020>";
					dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index].p_Get(t_i).p_UpdateWorldXY();
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1030>";
		var t_o=dbg_array(dbg_array(this.m__objectList,0)[dbg_index],0)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1031>";
		while((t_o)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1033>";
			t_o.p_UpdateWorldXY();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<1035>";
			t_o=dbg_array(dbg_array(this.m__objectList,0)[dbg_index],0)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iLayer.prototype.p_UpdateLayerObjects=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<927>";
	if(this.p_Grid()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<929>";
		var t_startX=((dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__cameraX/(this.m__blockWidth)-(this.m__blockWidthExtra))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<930>";
		var t_startY=((dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__cameraY/(this.m__blockHeight)-(this.m__blockHeightExtra))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<932>";
		if(t_startX<0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<932>";
			t_startX=0;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<933>";
		if(t_startY<0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<933>";
			t_startY=0;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<935>";
		var t_stopX=(((t_startX)+dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_VWidth()/(this.m__blockWidth)+1.0+1.0+(this.m__blockWidthExtra*2))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<936>";
		var t_stopY=(((t_startY)+dbg_object(bb_globals_iEnginePointer).m__currentPlayfield.p_VHeight()/(this.m__blockHeight)+1.0+1.0-(this.m__blockHeightExtra*2))|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<938>";
		if(t_stopX>this.p_GridWidth()){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<938>";
			t_stopX=this.p_GridWidth();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<939>";
		if(t_stopY>this.p_GridHeight()){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<939>";
			t_stopY=this.p_GridHeight();
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<941>";
		for(var t_y=t_startY;t_y<t_stopY;t_y=t_y+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<943>";
			for(var t_x=t_startX;t_x<t_stopX;t_x=t_x+1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<945>";
				var t_o=dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index].p_First();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<946>";
				while((t_o)!=null){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<948>";
					if(dbg_object(t_o).m__enabled==1){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<950>";
						t_o.p_Update();
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<951>";
						t_o.p_OnUpdate();
					}else{
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<955>";
						t_o.p_OnDisabled();
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<957>";
						if(dbg_object(t_o).m__enabled>1){
							err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<957>";
							dbg_object(t_o).m__enabled=dbg_object(t_o).m__enabled-1;
						}
					}
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<961>";
					t_o=dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index].p_Ascend();
				}
			}
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<971>";
		var t_o2=dbg_array(dbg_array(this.m__objectList,0)[dbg_index],0)[dbg_index].p_First();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<972>";
		while((t_o2)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<974>";
			if(dbg_object(t_o2).m__enabled==1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<976>";
				t_o2.p_Update();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<977>";
				t_o2.p_OnUpdate();
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<981>";
				t_o2.p_OnDisabled();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<983>";
				if(dbg_object(t_o2).m__enabled>1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<983>";
					dbg_object(t_o2).m__enabled=dbg_object(t_o2).m__enabled-1;
				}
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<987>";
			t_o2=dbg_array(dbg_array(this.m__objectList,0)[dbg_index],0)[dbg_index].p_Ascend();
		}
	}
	pop_err();
}
c_iLayer.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<637>";
	pop_err();
	return 0;
}
c_iLayer.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<914>";
	this.p_UpdateWorldXY();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<916>";
	this.p_UpdateLayerObjects();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<918>";
	this.p_OnUpdate();
	pop_err();
}
c_iLayer.prototype.p_Set2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<831>";
	if((bb_globals_iEnginePointer)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<831>";
		dbg_object(bb_globals_iEnginePointer).m__layerPointer=this;
	}
	pop_err();
}
c_iLayer.prototype.p_InitGrid=function(t_width,t_height){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<440>";
	this.m__objectList=resize_array_array(this.m__objectList,t_width);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<442>";
	for(var t_x=0;t_x<t_width;t_x=t_x+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<444>";
		if(dbg_array(this.m__objectList,t_x)[dbg_index].length<t_height){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<444>";
			dbg_array(this.m__objectList,t_x)[dbg_index]=resize_object_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_height);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<446>";
		for(var t_y=0;t_y<t_height;t_y=t_y+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<447>";
			if(!((dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index])!=null)){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<447>";
				dbg_array(dbg_array(this.m__objectList,t_x)[dbg_index],t_y)[dbg_index]=c_iList3.m_new.call(new c_iList3);
			}
		}
	}
	pop_err();
}
c_iLayer.prototype.p_OnCreate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<616>";
	pop_err();
	return 0;
}
c_iLayer.prototype.p_SystemInit=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<871>";
	this.p_Set2();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<873>";
	this.p_InitGrid(1,1);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<875>";
	this.p_OnCreate();
	pop_err();
}
c_iLayer.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<598>";
	c_iEngineObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<599>";
	this.p_SystemInit();
	pop_err();
	return this;
}
c_iLayer.prototype.p_AttachLast=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<142>";
	dbg_object(dbg_object(bb_globals_iEnginePointer).m__playfieldPointer).m__layerList.p_AddLast2(this);
	pop_err();
}
c_iLayer.prototype.p_AttachLast3=function(t_playfield){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<149>";
	dbg_object(t_playfield).m__layerList.p_AddLast2(this);
	pop_err();
}
c_iLayer.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<256>";
	this.m__loaderCache=null;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<258>";
	dbg_array(dbg_array(this.m__objectList,0)[dbg_index],0)[dbg_index]=null;
	pop_err();
}
c_iLayer.prototype.p_Remove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<671>";
	if(this.m__tattoo){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<673>";
		this.m__enabled=0;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<677>";
		this.m__enabled=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<679>";
		for(var t_i=0;t_i<dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_Length();t_i=t_i+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<680>";
			dbg_object(dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_Get(t_i)).m__layerList.p_Remove3(this);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<683>";
		if(this.m__autoDestroy){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layer.monkey<683>";
			c_iEngineObject.m_destroyList.p_AddLast4(this);
		}
	}
	pop_err();
}
function c_iList2(){
	Object.call(this);
	this.m__index=0;
	this.m__length=0;
	this.m__data=[];
}
c_iList2.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList2.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>";
	this.m__index=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
	if((this.m__length)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
		pop_err();
		return dbg_array(this.m__data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList2.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>";
	this.m__index=this.m__index+1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
	if(this.m__index>=0 && this.m__index<this.m__length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
		pop_err();
		return dbg_array(this.m__data,this.m__index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList2.prototype.p_AddLast2=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<55>";
	if(this.m__length==this.m__data.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<56>";
		this.m__data=resize_object_array(this.m__data,this.m__length*2+10);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<58>";
	dbg_array(this.m__data,this.m__length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<59>";
	this.m__length=this.m__length+1;
	pop_err();
}
c_iList2.prototype.p_Position4=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>";
	for(var t_i=0;t_i<this.m__length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
		if(dbg_array(this.m__data,t_i)[dbg_index]==t_value){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
			pop_err();
			return t_i;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>";
	pop_err();
	return -1;
}
c_iList2.prototype.p_RemoveFromIndex=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>";
	for(var t_i=t_index;t_i<this.m__length-1;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>";
		dbg_array(this.m__data,t_i)[dbg_index]=dbg_array(this.m__data,t_i+1)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>";
	this.m__length=this.m__length-1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>";
	this.m__index=this.m__index-1;
	pop_err();
}
c_iList2.prototype.p_Remove3=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>";
	var t_p=this.p_Position4(t_value);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
	if(t_p>=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
		this.p_RemoveFromIndex(t_p);
	}
	pop_err();
}
function c_iGraph(){
	c_iEngineObject.call(this);
	this.m__visible=true;
	this.m__ghost=false;
	this.m__height=0;
	this.m__scaleY=1.0;
	this.m__scaleX=1.0;
	this.m__rotation=.0;
	this.m__width=0;
	this.m__x=.0;
	this.m__y=.0;
	this.m__z=.0;
	this.m__mcPosition=c_iVector2d.m_new.call(new c_iVector2d,0.0,0.0);
	this.m__mcVelocity=c_iVector2d.m_new.call(new c_iVector2d,0.0,0.0);
	this.m__ghostBlend=0;
	this.m__ghostAlpha=1.0;
	this.m__ghostRed=255.0;
	this.m__ghostGreen=255.0;
	this.m__ghostBlue=255.0;
	this.m__blend=0;
	this.m__alpha=1.0;
	this.m__red=255.0;
	this.m__green=255.0;
	this.m__blue=255.0;
	this.implments={c_IMover:1};
}
c_iGraph.prototype=extend_class(c_iEngineObject);
c_iGraph.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<480>";
	var t_=(this.m__height)*this.m__scaleY;
	pop_err();
	return t_;
}
c_iGraph.prototype.p_Height2=function(t_height){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<487>";
	this.m__height=((t_height)|0);
	pop_err();
}
c_iGraph.prototype.p_ScaleX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<628>";
	pop_err();
	return this.m__scaleX;
}
c_iGraph.prototype.p_ScaleX2=function(t_scaleX){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<635>";
	this.m__scaleX=t_scaleX;
	pop_err();
}
c_iGraph.prototype.p_Rotation=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<594>";
	pop_err();
	return this.m__rotation;
}
c_iGraph.prototype.p_Rotation2=function(t_angle){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<601>";
	this.m__rotation=t_angle;
	pop_err();
}
c_iGraph.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<670>";
	var t_=(this.m__width)*this.m__scaleX;
	pop_err();
	return t_;
}
c_iGraph.prototype.p_Width2=function(t_width){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<677>";
	this.m__width=((t_width)|0);
	pop_err();
}
c_iGraph.prototype.p_PositionX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<494>";
	pop_err();
	return this.m__x;
}
c_iGraph.prototype.p_PositionX2=function(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<501>";
	this.m__x=t_x;
	pop_err();
}
c_iGraph.prototype.p_PositionX3=function(t_x,t_graph){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<508>";
	this.m__x=dbg_object(t_graph).m__x+t_x;
	pop_err();
}
c_iGraph.prototype.p_PositionY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<515>";
	pop_err();
	return this.m__y;
}
c_iGraph.prototype.p_PositionY2=function(t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<522>";
	this.m__y=t_y;
	pop_err();
}
c_iGraph.prototype.p_PositionY3=function(t_y,t_graph){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<529>";
	this.m__y=dbg_object(t_graph).m__y+t_y;
	pop_err();
}
c_iGraph.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<10>";
	c_iEngineObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<10>";
	pop_err();
	return this;
}
c_iGraph.prototype.p_PositionZ=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<536>";
	pop_err();
	return this.m__z;
}
c_iGraph.prototype.p_PositionZ2=function(t_z){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<543>";
	this.m__z=t_z;
	pop_err();
}
c_iGraph.prototype.p_PositionZ3=function(t_z,t_graph){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<550>";
	this.m__z=dbg_object(t_graph).m__z+t_z;
	pop_err();
}
c_iGraph.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<347>";
	this.m__mcPosition=null;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<348>";
	this.m__mcVelocity=null;
	pop_err();
}
c_iGraph.prototype.p_ScaleY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<642>";
	pop_err();
	return this.m__scaleY;
}
c_iGraph.prototype.p_ScaleY2=function(t_scaleY){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/graph.monkey<649>";
	this.m__scaleY=t_scaleY;
	pop_err();
}
function c_iLayerObject(){
	c_iGraph.call(this);
	this.m__worldHeight=.0;
	this.m__worldScaleX=.0;
	this.m__worldScaleY=.0;
	this.m__worldRotation=.0;
	this.m__worldWidth=.0;
	this.m__worldX=.0;
	this.m__worldY=.0;
	this.m__parent=null;
	this.m__layer=c_iLayerObject.m_AutoLayer();
	this.m__scoreCollector=null;
	this.m__cosine=.0;
	this.m__cosineRadius=.0;
	this.m__sine=.0;
	this.m__sineRadius=.0;
	this.m__stamina=1;
	this.m__invincible=false;
	this.m__hits=0;
	this.m__scoreCount=false;
	this.m__scoreBoard=0;
	this.m__points=0;
	this.m__countDown=false;
	this.m__collisionRead=false;
	this.m__collisionWrite=false;
	this.m__column=0;
	this.m__row=0;
	this.m__debugInfo=new_string_array(8);
	this.m__loaderCache=null;
	this.m__collisionMask=[];
	this.m__control=[];
	this.implments={c_IMover:1};
}
c_iLayerObject.prototype=extend_class(c_iGraph);
c_iLayerObject.prototype.p_Render=function(){
	push_err();
	pop_err();
}
c_iLayerObject.prototype.p_UpdateWorldXY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2604>";
	this.m__worldHeight=bb_math_Abs2(this.p_Height());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2605>";
	this.m__worldScaleX=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2606>";
	this.m__worldScaleY=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2607>";
	this.m__worldRotation=this.p_Rotation();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2608>";
	this.m__worldWidth=bb_math_Abs2(this.p_Width());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2609>";
	this.m__worldX=this.m__x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2610>";
	this.m__worldY=this.m__y;
	pop_err();
}
c_iLayerObject.prototype.p_Update=function(){
	push_err();
	pop_err();
}
c_iLayerObject.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1693>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_OnDisabled=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1613>";
	pop_err();
	return 0;
}
c_iLayerObject.m_AutoLayer=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2749>";
	if(!((bb_globals_iEnginePointer)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2749>";
		c_iEngine.m_new.call(new c_iEngine);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2750>";
	if(!((dbg_object(bb_globals_iEnginePointer).m__playfieldPointer)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2750>";
		c_iPlayfield.m_new.call(new c_iPlayfield);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2751>";
	if(!((dbg_object(bb_globals_iEnginePointer).m__layerPointer)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2751>";
		c_iLayer.m_new.call(new c_iLayer);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2753>";
	pop_err();
	return dbg_object(bb_globals_iEnginePointer).m__layerPointer;
}
c_iLayerObject.prototype.p_Layer=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1126>";
	if((this.m__parent)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1127>";
		pop_err();
		return dbg_object(this.m__parent).m__layer;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1129>";
		pop_err();
		return this.m__layer;
	}
}
c_iLayerObject.prototype.p_Layer2=function(t_layer){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1138>";
	this.m__layer=t_layer;
	pop_err();
}
c_iLayerObject.prototype.p_Cosine=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<832>";
	pop_err();
	return this.m__cosine;
}
c_iLayerObject.prototype.p_Cosine2=function(t_cosine){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<839>";
	this.m__cosine=t_cosine;
	pop_err();
}
c_iLayerObject.prototype.p_CosineRadius=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<846>";
	pop_err();
	return this.m__cosineRadius;
}
c_iLayerObject.prototype.p_CosineRadius2=function(t_cosineRadius){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<853>";
	this.m__cosineRadius=t_cosineRadius;
	pop_err();
}
c_iLayerObject.prototype.p_RenderX=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1950>";
	var t_=this.p_PositionX()+Math.cos((this.p_Cosine())*D2R)*this.p_CosineRadius();
	pop_err();
	return t_;
}
c_iLayerObject.prototype.p_Sine=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2085>";
	pop_err();
	return this.m__sine;
}
c_iLayerObject.prototype.p_Sine2=function(t_sine){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2092>";
	this.m__sine=t_sine;
	pop_err();
}
c_iLayerObject.prototype.p_SineRadius=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2099>";
	pop_err();
	return this.m__sineRadius;
}
c_iLayerObject.prototype.p_SineRadius2=function(t_sineRadius){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2106>";
	this.m__sineRadius=t_sineRadius;
	pop_err();
}
c_iLayerObject.prototype.p_RenderY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1957>";
	var t_=this.p_PositionY()+Math.sin((this.p_Sine())*D2R)*this.p_SineRadius();
	pop_err();
	return t_;
}
c_iLayerObject.prototype.p_CollisionMethod=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<657>";
	if(c_iCollision.m_Rectangles(this.p_RenderX(),this.p_RenderY(),((this.p_Width())|0),((this.p_Height())|0),t_layerObject.p_RenderX(),t_layerObject.p_RenderY(),((t_layerObject.p_Width())|0),((t_layerObject.p_Height())|0),1)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<657>";
		pop_err();
		return 1;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<658>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_Collides=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<639>";
	if(this.m__enabled==1 && dbg_object(t_layerObject).m__enabled==1 && t_layerObject!=this && t_layerObject!=this.m__scoreCollector){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<640>";
		var t_=this.p_CollisionMethod(t_layerObject);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<643>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_OnOutro=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1622>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_OnRemove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1670>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_Remove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1809>";
	if((this.m__layer)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1811>";
		if(this.m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1813>";
			if(this.m__countDown){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1813>";
				dbg_object(bb_globals_iEnginePointer).m__countDown=dbg_object(bb_globals_iEnginePointer).m__countDown-1;
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1815>";
			this.m__enabled=0;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1819>";
		if(this.m__tattoo==false){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1821>";
			if(this.m__collisionRead){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1822>";
				dbg_object(bb_globals_iEnginePointer).m__collisionReadList.p_Remove4(this);
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1823>";
				this.m__collisionRead=false;
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1826>";
			if(this.m__collisionWrite){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1827>";
				dbg_object(bb_globals_iEnginePointer).m__collisionWriteList.p_Remove4(this);
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1828>";
				this.m__collisionWrite=false;
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1831>";
			dbg_array(dbg_array(dbg_object(this.m__layer).m__objectList,this.m__column)[dbg_index],this.m__row)[dbg_index].p_Remove4(this);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1833>";
			this.p_OnRemove();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1835>";
			if(this.m__autoDestroy){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1835>";
				c_iEngineObject.m_destroyList.p_AddLast4(this);
			}
		}
	}
	pop_err();
}
c_iLayerObject.prototype.p_ScoreSystem=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2136>";
	var t_s1=this.m__stamina;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2137>";
	var t_s2=dbg_object(t_layerObject).m__stamina;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2139>";
	if(this.m__invincible){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2140>";
		if(dbg_object(t_layerObject).m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2140>";
			dbg_object(t_layerObject).m__stamina=0;
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2142>";
		if(dbg_object(t_layerObject).m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2143>";
			this.m__stamina=this.m__stamina-t_s2;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2144>";
			this.m__hits=this.m__hits+1;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2145>";
			this.m__ghost=true;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2149>";
	if(dbg_object(t_layerObject).m__invincible){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2150>";
		if(this.m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2150>";
			this.m__stamina=0;
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2152>";
		if(this.m__enabled==1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2153>";
			dbg_object(t_layerObject).m__stamina=dbg_object(t_layerObject).m__stamina-t_s1;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2154>";
			dbg_object(t_layerObject).m__hits=dbg_object(t_layerObject).m__hits+1;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2155>";
			dbg_object(t_layerObject).m__ghost=true;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2159>";
	if(this.m__stamina<1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2161>";
		this.m__stamina=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2163>";
		if(this.m__scoreCount){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2163>";
			dbg_object(bb_globals_iEnginePointer).m__scoreCount=dbg_object(bb_globals_iEnginePointer).m__scoreCount+1;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2165>";
		if((dbg_object(t_layerObject).m__scoreCollector)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2166>";
			dbg_object(dbg_object(t_layerObject).m__scoreCollector).m__scoreBoard=dbg_object(dbg_object(t_layerObject).m__scoreCollector).m__scoreBoard+this.m__points;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2168>";
			dbg_object(t_layerObject).m__scoreBoard=dbg_object(t_layerObject).m__scoreBoard+this.m__points;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2171>";
		this.p_OnOutro();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2172>";
		this.p_Remove();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2176>";
	if(dbg_object(t_layerObject).m__stamina<1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2178>";
		dbg_object(t_layerObject).m__stamina=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2180>";
		if(dbg_object(t_layerObject).m__scoreCount){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2180>";
			dbg_object(bb_globals_iEnginePointer).m__scoreCount=dbg_object(bb_globals_iEnginePointer).m__scoreCount+1;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2182>";
		if((this.m__scoreCollector)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2183>";
			dbg_object(this.m__scoreCollector).m__scoreBoard=dbg_object(this.m__scoreCollector).m__scoreBoard+dbg_object(t_layerObject).m__points;
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2185>";
			this.m__scoreBoard=this.m__scoreBoard+dbg_object(t_layerObject).m__points;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2188>";
		t_layerObject.p_OnOutro();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2189>";
		t_layerObject.p_Remove();
	}
	pop_err();
}
c_iLayerObject.prototype.p_OnCollision=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1605>";
	this.p_ScoreSystem(t_layerObject);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1606>";
	pop_err();
	return 0;
}
c_iLayerObject.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<13>";
	c_iGraph.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<13>";
	pop_err();
	return this;
}
c_iLayerObject.prototype.p_OnAttach=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1596>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_Attach=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<507>";
	this.p_OnAttach();
	pop_err();
}
c_iLayerObject.prototype.p_AttachLast=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<461>";
	if((this.m__layer)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<462>";
		dbg_array(dbg_array(dbg_object(this.m__layer).m__objectList,this.m__column)[dbg_index],this.m__row)[dbg_index].p_Remove4(this);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<463>";
		this.m__column=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<464>";
		this.m__row=0;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<467>";
	dbg_array(dbg_array(dbg_object(dbg_object(bb_globals_iEnginePointer).m__layerPointer).m__objectList,this.m__column)[dbg_index],this.m__row)[dbg_index].p_AddLast3(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<468>";
	this.m__layer=dbg_object(bb_globals_iEnginePointer).m__layerPointer;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<470>";
	this.p_Attach();
	pop_err();
}
c_iLayerObject.prototype.p_AttachLast4=function(t_layer){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<479>";
	if((this.m__layer)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<480>";
		dbg_array(dbg_array(dbg_object(this.m__layer).m__objectList,this.m__column)[dbg_index],this.m__row)[dbg_index].p_Remove4(this);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<481>";
		this.m__column=0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<482>";
		this.m__row=0;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<485>";
	dbg_array(dbg_array(dbg_object(t_layer).m__objectList,this.m__column)[dbg_index],this.m__row)[dbg_index].p_AddLast3(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<486>";
	this.m__layer=t_layer;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<488>";
	this.p_Attach();
	pop_err();
}
c_iLayerObject.prototype.p_Parent=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1042>";
	pop_err();
	return this.m__parent;
}
c_iLayerObject.prototype.p_Parent2=function(t_parent){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1049>";
	this.m__parent=t_parent;
	pop_err();
}
c_iLayerObject.prototype.p_AttachLast5=function(t_layerGroup){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<497>";
	t_layerGroup.p_AddItemLast(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<499>";
	this.p_Attach();
	pop_err();
}
c_iLayerObject.prototype.p_Position=function(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1743>";
	this.m__x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1744>";
	this.m__y=t_y;
	pop_err();
}
c_iLayerObject.prototype.p_Position5=function(t_x,t_y,t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1753>";
	this.m__x=dbg_object(t_layerObject).m__x+t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1754>";
	this.m__y=dbg_object(t_layerObject).m__y+t_y;
	pop_err();
}
c_iLayerObject.prototype.p_Position2=function(t_x,t_y,t_z){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1763>";
	this.m__x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1764>";
	this.m__y=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1765>";
	this.m__z=t_z;
	pop_err();
}
c_iLayerObject.prototype.p_Position6=function(t_x,t_y,t_z,t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1774>";
	this.m__x=t_layerObject.p_PositionX()+t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1775>";
	this.m__y=t_layerObject.p_PositionY()+t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1776>";
	this.m__z=t_layerObject.p_PositionZ()+t_z;
	pop_err();
}
c_iLayerObject.prototype.p_Show=function(t_info){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2122>";
	for(var t_i=0;t_i<this.m__debugInfo.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2123>";
		if(!((dbg_array(this.m__debugInfo,t_i)[dbg_index]).length!=0)){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2124>";
			dbg_array(this.m__debugInfo,t_i)[dbg_index]=t_info;
			pop_err();
			return;
		}
	}
	pop_err();
}
c_iLayerObject.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<861>";
	c_iGraph.prototype.p_Destroy.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<863>";
	this.m__loaderCache=null;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<865>";
	this.m__collisionMask=[];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<866>";
	this.m__control=[];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<867>";
	this.m__debugInfo=[];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<868>";
	this.m__layer=null;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<869>";
	this.m__parent=null;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<870>";
	this.m__scoreCollector=null;
	pop_err();
}
c_iLayerObject.prototype.p_SetAlpha=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2049>";
	bb_graphics_SetAlpha(t_alpha*dbg_object(bb_globals_iEnginePointer).m__alphaFade*dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__alphaFade*dbg_object(this.m__layer).m__alphaFade);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2051>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_SetColor=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2063>";
	bb_graphics_SetColor(t_red*dbg_object(bb_globals_iEnginePointer).m__colorFade*dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__colorFade*dbg_object(this.m__layer).m__colorFade,t_green*dbg_object(bb_globals_iEnginePointer).m__colorFade*dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__colorFade*dbg_object(this.m__layer).m__colorFade,t_blue*dbg_object(bb_globals_iEnginePointer).m__colorFade*dbg_object(dbg_object(bb_globals_iEnginePointer).m__currentPlayfield).m__colorFade*dbg_object(this.m__layer).m__colorFade);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2065>";
	pop_err();
	return 0;
}
c_iLayerObject.prototype.p_RenderDebugInfo=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1883>";
	if(((this.m__debugInfo).length!=0) && ((dbg_array(this.m__debugInfo,0)[dbg_index]).length!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1885>";
		bb_graphics_SetBlend(0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1886>";
		this.p_SetAlpha(1.0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1887>";
		this.p_SetColor(255.0,255.0,255.0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1889>";
		for(var t_i=0;t_i<this.m__debugInfo.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1890>";
			if((dbg_array(this.m__debugInfo,t_i)[dbg_index]).length!=0){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1891>";
				bb_graphics_DrawText(dbg_array(this.m__debugInfo,t_i)[dbg_index],this.p_RenderX()-this.p_Width()*.5,this.p_RenderY()+this.p_Height()*.5+(t_i*12),0.0,0.0);
			}
		}
	}
	pop_err();
}
c_iLayerObject.prototype.p_ClearDebugInfo=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<544>";
	if(((this.m__debugInfo).length!=0) && ((dbg_array(this.m__debugInfo,0)[dbg_index]).length!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<545>";
		for(var t_i=0;t_i<this.m__debugInfo.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<546>";
			dbg_array(this.m__debugInfo,t_i)[dbg_index]="";
		}
	}
	pop_err();
}
c_iLayerObject.prototype.p_RenderObject=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1921>";
	dbg_object(t_layerObject).m__layer=this.m__layer;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1923>";
	if(dbg_object(t_layerObject).m__enabled==1 && dbg_object(t_layerObject).m__visible){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1925>";
		bb_graphics_PushMatrix();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1928>";
		bb_graphics_Translate(this.p_RenderX(),this.p_RenderY());
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1929>";
		bb_graphics_Rotate(this.p_Rotation()*(bb_globals_iRotation));
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1930>";
		bb_graphics_Scale(this.p_ScaleX(),this.p_ScaleY());
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1932>";
		var t_alpha=dbg_object(t_layerObject).m__alpha;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1934>";
		dbg_object(t_layerObject).m__alpha=dbg_object(t_layerObject).m__alpha*this.m__alpha;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1936>";
		t_layerObject.p_Render();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1938>";
		dbg_object(t_layerObject).m__alpha=this.m__alpha;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1940>";
		bb_graphics_PopMatrix();
	}
	pop_err();
}
c_iLayerObject.prototype.p_RenderList=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1904>";
	var t_o=t_list.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1906>";
	while((t_o)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1908>";
		this.p_RenderObject(t_o);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<1910>";
		t_o=t_list.p_Ascend();
	}
	pop_err();
}
c_iLayerObject.prototype.p_TestForRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2330>";
	if(this.m__scaleX==0.0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2330>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2331>";
	if(this.m__scaleY==0.0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2331>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2332>";
	if(this.m__alpha==0.0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2332>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2334>";
	pop_err();
	return true;
}
c_iLayerObject.prototype.p_UpdateObject=function(t_LayerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2524>";
	dbg_object(t_LayerObject).m__layer=this.m__layer;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2526>";
	if(this.p_TestForRender()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2528>";
		dbg_object(t_LayerObject).m__x=dbg_object(t_LayerObject).m__x*this.p_ScaleX();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2529>";
		dbg_object(t_LayerObject).m__y=dbg_object(t_LayerObject).m__y*this.p_ScaleY();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2531>";
		dbg_object(t_LayerObject).m__x=dbg_object(t_LayerObject).m__x+this.p_RenderX();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2532>";
		dbg_object(t_LayerObject).m__y=dbg_object(t_LayerObject).m__y+this.p_RenderY();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2534>";
		dbg_object(t_LayerObject).m__rotation=dbg_object(t_LayerObject).m__rotation+this.m__rotation;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2536>";
		dbg_object(t_LayerObject).m__scaleX=dbg_object(t_LayerObject).m__scaleX*this.m__scaleX;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2537>";
		dbg_object(t_LayerObject).m__scaleY=dbg_object(t_LayerObject).m__scaleY*this.m__scaleY;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2538>";
		dbg_object(t_LayerObject).m__alpha=dbg_object(t_LayerObject).m__alpha*this.m__alpha;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2540>";
		if(this.m__rotation==0.0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2542>";
			if(dbg_object(t_LayerObject).m__enabled==1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2544>";
				t_LayerObject.p_Update();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2545>";
				t_LayerObject.p_OnUpdate();
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2549>";
				t_LayerObject.p_OnDisabled();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2551>";
				if(dbg_object(t_LayerObject).m__enabled>1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2551>";
					dbg_object(t_LayerObject).m__enabled=dbg_object(t_LayerObject).m__enabled-1;
				}
			}
		}else{
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2557>";
			var t_a=(Math.atan2(dbg_object(t_LayerObject).m__y-this.m__y,dbg_object(t_LayerObject).m__x-this.m__x)*R2D)+this.m__rotation*(-bb_globals_iRotation);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2558>";
			var t_d=Math.sqrt(Math.pow(dbg_object(t_LayerObject).m__y-this.m__y,2.0)+Math.pow(dbg_object(t_LayerObject).m__x-this.m__x,2.0));
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2559>";
			var t_tx=this.m__x+Math.cos((t_a)*D2R)*t_d;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2560>";
			var t_ty=this.m__y+Math.sin((t_a)*D2R)*t_d;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2561>";
			t_tx=t_tx-dbg_object(t_LayerObject).m__x;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2562>";
			t_ty=t_ty-dbg_object(t_LayerObject).m__y;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2563>";
			dbg_object(t_LayerObject).m__x=dbg_object(t_LayerObject).m__x+t_tx;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2564>";
			dbg_object(t_LayerObject).m__y=dbg_object(t_LayerObject).m__y+t_ty;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2566>";
			if(dbg_object(t_LayerObject).m__enabled==1){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2568>";
				t_LayerObject.p_Update();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2569>";
				t_LayerObject.p_OnUpdate();
			}else{
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2573>";
				t_LayerObject.p_OnDisabled();
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2575>";
				if(dbg_object(t_LayerObject).m__enabled>1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2575>";
					dbg_object(t_LayerObject).m__enabled=dbg_object(t_LayerObject).m__enabled-1;
				}
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2579>";
			dbg_object(t_LayerObject).m__x=dbg_object(t_LayerObject).m__x-t_tx;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2580>";
			dbg_object(t_LayerObject).m__y=dbg_object(t_LayerObject).m__y-t_ty;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2584>";
		dbg_object(t_LayerObject).m__x=dbg_object(t_LayerObject).m__x-this.p_RenderX();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2585>";
		dbg_object(t_LayerObject).m__y=dbg_object(t_LayerObject).m__y-this.p_RenderY();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2587>";
		dbg_object(t_LayerObject).m__x=dbg_object(t_LayerObject).m__x/this.p_ScaleX();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2588>";
		dbg_object(t_LayerObject).m__y=dbg_object(t_LayerObject).m__y/this.p_ScaleY();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2590>";
		dbg_object(t_LayerObject).m__rotation=dbg_object(t_LayerObject).m__rotation-this.m__rotation;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2592>";
		dbg_object(t_LayerObject).m__scaleX=dbg_object(t_LayerObject).m__scaleX/this.m__scaleX;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2593>";
		dbg_object(t_LayerObject).m__scaleY=dbg_object(t_LayerObject).m__scaleY/this.m__scaleY;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2594>";
		dbg_object(t_LayerObject).m__alpha=dbg_object(t_LayerObject).m__alpha/this.m__alpha;
	}
	pop_err();
}
c_iLayerObject.prototype.p_UpdateList=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2508>";
	var t_o=t_list.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2509>";
	while((t_o)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2511>";
		this.p_UpdateObject(t_o);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2513>";
		t_o=t_list.p_Ascend();
	}
	pop_err();
}
c_iLayerObject.prototype.p_UpdateWorldXYObject=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2634>";
	var t_x=dbg_object(t_layerObject).m__x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2635>";
	var t_y=dbg_object(t_layerObject).m__y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2636>";
	var t_rotation=dbg_object(t_layerObject).m__rotation;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2637>";
	var t_scaleX=dbg_object(t_layerObject).m__scaleX;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2638>";
	var t_scaleY=dbg_object(t_layerObject).m__scaleY;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2640>";
	dbg_object(t_layerObject).m__x=dbg_object(t_layerObject).m__x*this.p_ScaleX();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2641>";
	dbg_object(t_layerObject).m__y=dbg_object(t_layerObject).m__y*this.p_ScaleY();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2642>";
	dbg_object(t_layerObject).m__x=dbg_object(t_layerObject).m__x+this.p_PositionX();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2643>";
	dbg_object(t_layerObject).m__y=dbg_object(t_layerObject).m__y+this.p_PositionY();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2644>";
	dbg_object(t_layerObject).m__rotation=dbg_object(t_layerObject).m__rotation+this.m__rotation;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2645>";
	dbg_object(t_layerObject).m__scaleX=dbg_object(t_layerObject).m__scaleX*this.m__scaleX;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2646>";
	dbg_object(t_layerObject).m__scaleY=dbg_object(t_layerObject).m__scaleY*this.m__scaleY;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2648>";
	var t_a=(Math.atan2(dbg_object(t_layerObject).m__y-this.m__y,dbg_object(t_layerObject).m__x-this.m__x)*R2D)+this.m__rotation*(-bb_globals_iRotation);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2649>";
	var t_d=Math.sqrt(Math.pow(dbg_object(t_layerObject).m__y-this.m__y,2.0)+Math.pow(dbg_object(t_layerObject).m__x-this.m__x,2.0));
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2650>";
	var t_tx=this.m__x+Math.cos((t_a)*D2R)*t_d;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2651>";
	var t_ty=this.m__y+Math.sin((t_a)*D2R)*t_d;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2652>";
	t_tx=t_tx-dbg_object(t_layerObject).m__x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2653>";
	t_ty=t_ty-dbg_object(t_layerObject).m__y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2654>";
	dbg_object(t_layerObject).m__x=dbg_object(t_layerObject).m__x+t_tx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2655>";
	dbg_object(t_layerObject).m__y=dbg_object(t_layerObject).m__y+t_ty;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2657>";
	t_layerObject.p_UpdateWorldXY();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2659>";
	dbg_object(t_layerObject).m__x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2660>";
	dbg_object(t_layerObject).m__y=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2661>";
	dbg_object(t_layerObject).m__rotation=t_rotation;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2662>";
	dbg_object(t_layerObject).m__scaleX=t_scaleX;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2663>";
	dbg_object(t_layerObject).m__scaleY=t_scaleY;
	pop_err();
}
c_iLayerObject.prototype.p_UpdateWorldXYList=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2618>";
	var t_o=t_list.p_First();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2619>";
	while((t_o)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2621>";
		this.p_UpdateWorldXYObject(t_o);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layerobject.monkey<2623>";
		t_o=t_list.p_Ascend();
	}
	pop_err();
}
function c_iList3(){
	Object.call(this);
	this.m__length=0;
	this.m__data=[];
	this.m__index=0;
}
c_iList3.prototype.p_Length=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<170>";
	pop_err();
	return this.m__length;
}
c_iList3.prototype.p_Get=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<128>";
	pop_err();
	return dbg_array(this.m__data,t_index)[dbg_index];
}
c_iList3.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>";
	this.m__index=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
	if((this.m__length)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
		pop_err();
		return dbg_array(this.m__data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList3.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>";
	this.m__index=this.m__index+1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
	if(this.m__index>=0 && this.m__index<this.m__length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
		pop_err();
		return dbg_array(this.m__data,this.m__index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList3.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList3.prototype.p_Position7=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>";
	for(var t_i=0;t_i<this.m__length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
		if(dbg_array(this.m__data,t_i)[dbg_index]==t_value){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
			pop_err();
			return t_i;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>";
	pop_err();
	return -1;
}
c_iList3.prototype.p_RemoveFromIndex=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>";
	for(var t_i=t_index;t_i<this.m__length-1;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>";
		dbg_array(this.m__data,t_i)[dbg_index]=dbg_array(this.m__data,t_i+1)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>";
	this.m__length=this.m__length-1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>";
	this.m__index=this.m__index-1;
	pop_err();
}
c_iList3.prototype.p_Remove4=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>";
	var t_p=this.p_Position7(t_value);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
	if(t_p>=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
		this.p_RemoveFromIndex(t_p);
	}
	pop_err();
}
c_iList3.prototype.p_AddLast3=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<55>";
	if(this.m__length==this.m__data.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<56>";
		this.m__data=resize_object_array(this.m__data,this.m__length*2+10);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<58>";
	dbg_array(this.m__data,this.m__length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<59>";
	this.m__length=this.m__length+1;
	pop_err();
}
function c_iGuiObject(){
	c_iGraph.call(this);
	this.m__worldHeight=.0;
	this.m__worldScaleX=.0;
	this.m__worldScaleY=.0;
	this.m__worldRotation=.0;
	this.m__worldWidth=.0;
	this.m__worldX=.0;
	this.m__worldY=.0;
	this.m__multiTouch=false;
	this.m__touchIndex=0;
	this.m__parent=null;
	this.m__playfield=c_iGuiObject.m_AutoPlayfield();
	this.m__handleX=0;
	this.m__handleY=0;
	this.m__touchDown=0;
	this.m__over=false;
	this.m__rememberTopObject=true;
	this.m__debugInfo=new_string_array(8);
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
c_iGuiObject.m__topObject=null;
c_iGuiObject.prototype.p_UpdateWorldXY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2018>";
	this.m__worldHeight=bb_math_Abs2(this.p_Height());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2019>";
	this.m__worldScaleX=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2020>";
	this.m__worldScaleY=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2021>";
	this.m__worldRotation=this.p_Rotation();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2022>";
	this.m__worldWidth=bb_math_Abs2(this.p_Width());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2023>";
	this.m__worldX=this.m__x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2024>";
	this.m__worldY=this.m__y;
	pop_err();
}
c_iGuiObject.prototype.p_Parent=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1193>";
	pop_err();
	return this.m__parent;
}
c_iGuiObject.prototype.p_Parent3=function(t_parent){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1200>";
	this.m__parent=t_parent;
	pop_err();
}
c_iGuiObject.m_AutoPlayfield=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2163>";
	if(!((bb_globals_iEnginePointer)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2163>";
		c_iEngine.m_new.call(new c_iEngine);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2164>";
	if(!((dbg_object(bb_globals_iEnginePointer).m__playfieldPointer)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2164>";
		c_iPlayfield.m_new.call(new c_iPlayfield);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<2166>";
	pop_err();
	return dbg_object(bb_globals_iEnginePointer).m__playfieldPointer;
}
c_iGuiObject.prototype.p_Playfield=function(t_playfield){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1344>";
	this.m__playfield=t_playfield;
	pop_err();
}
c_iGuiObject.prototype.p_Playfield2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1332>";
	if((this.p_Parent())!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1333>";
		var t_=this.p_Parent().p_Playfield2();
		pop_err();
		return t_;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1335>";
		pop_err();
		return this.m__playfield;
	}
}
c_iGuiObject.prototype.p_TouchX=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1724>";
	if(!((bb_globals_iEnginePointer)!=null) || !((this.p_Playfield2())!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1724>";
		var t_=bb_input_TouchX(t_index);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1726>";
	var t_2=bb_input_TouchX(t_index)/dbg_object(this.p_Playfield2()).m__scaleX-dbg_object(this.p_Playfield2()).m__x/dbg_object(this.p_Playfield2()).m__scaleX;
	pop_err();
	return t_2;
}
c_iGuiObject.prototype.p_TouchY=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1735>";
	if(!((bb_globals_iEnginePointer)!=null) || !((this.p_Playfield2())!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1735>";
		var t_=bb_input_TouchY(t_index);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1737>";
	var t_2=bb_input_TouchY(t_index)/dbg_object(this.p_Playfield2()).m__scaleY-dbg_object(this.p_Playfield2()).m__y/dbg_object(this.p_Playfield2()).m__scaleY;
	pop_err();
	return t_2;
}
c_iGuiObject.prototype.p_TouchOver=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1711>";
	if(this.p_Playfield2()!=(c_iSystemGui.m__playfield) && this.p_Playfield2()!=bb_functions_iGetTouchPlayfield(t_index)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1711>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1713>";
	if(bb_commoncode_iPointInsideRectange(this.m__worldX,this.m__worldY,this.m__worldWidth,this.m__worldHeight,this.p_TouchX(t_index),this.p_TouchY(t_index),this.m__worldRotation*(bb_globals_iRotation),(this.m__handleX)*bb_math_Abs2(this.m__worldScaleX),(this.m__handleY)*bb_math_Abs2(this.m__worldScaleY))){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1713>";
		pop_err();
		return true;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1715>";
	pop_err();
	return false;
}
c_iGuiObject.prototype.p_GetTouchIndex=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<853>";
	for(this.m__touchIndex=0;this.m__touchIndex<bb_functions_iTouchDowns();this.m__touchIndex=this.m__touchIndex+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<854>";
		if(((bb_input_TouchDown(this.m__touchIndex))!=0) && this.p_TouchOver(this.m__touchIndex)){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<854>";
			pop_err();
			return true;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<857>";
	pop_err();
	return false;
}
c_iGuiObject.prototype.p_UpdateInput=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1945>";
	if(this.m__multiTouch){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1945>";
		this.p_GetTouchIndex();
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1947>";
	this.m__touchDown=bb_input_TouchDown(this.m__touchIndex);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1949>";
	this.m__over=this.p_TouchOver(this.m__touchIndex);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1951>";
	if(this.m__rememberTopObject){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1953>";
		if(!this.m__multiTouch && ((c_iGuiObject.m__topObject)!=null)){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1953>";
			this.m__over=false;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1955>";
		if(!((c_iGuiObject.m__topObject)!=null) && this.m__over){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1955>";
			c_iGuiObject.m__topObject=this;
		}
	}
	pop_err();
}
c_iGuiObject.prototype.p_UpdateGhost=function(){
	push_err();
	pop_err();
}
c_iGuiObject.prototype.p_ClearDebugInfo=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<749>";
	if(((this.m__debugInfo).length!=0) && ((dbg_array(this.m__debugInfo,0)[dbg_index]).length!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<750>";
		for(var t_i=0;t_i<this.m__debugInfo.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<751>";
			dbg_array(this.m__debugInfo,t_i)[dbg_index]="";
		}
	}
	pop_err();
}
c_iGuiObject.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1821>";
	this.p_ClearDebugInfo();
	pop_err();
}
c_iGuiObject.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1170>";
	pop_err();
	return 0;
}
c_iGuiObject.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<775>";
	c_iGraph.prototype.p_Destroy.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<777>";
	this.m__parent=null;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<778>";
	this.m__playfield=null;
	pop_err();
}
c_iGuiObject.prototype.p_OnRemove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1160>";
	pop_err();
	return 0;
}
c_iGuiObject.prototype.p_Remove=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1425>";
	if(dbg_object(this.p_Playfield2()).m__guiLastObject==this){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1425>";
		dbg_object(this.p_Playfield2()).m__guiLastObject=null;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1427>";
	this.p_Disable();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1429>";
	if(this.m__tattoo==false){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1431>";
		this.p_OnRemove();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1433>";
		for(var t_i=0;t_i<dbg_object(this.p_Playfield2()).m__guiList.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1434>";
			dbg_array(dbg_object(this.p_Playfield2()).m__guiList,t_i)[dbg_index].p_Remove5(this);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1437>";
		if(this.m__autoDestroy){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/guiobject.monkey<1437>";
			c_iEngineObject.m_destroyList.p_AddLast4(this);
		}
	}
	pop_err();
}
function c_iList4(){
	Object.call(this);
	this.m__index=0;
	this.m__length=0;
	this.m__data=[];
}
c_iList4.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>";
	this.m__index=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
	if((this.m__length)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
		pop_err();
		return dbg_array(this.m__data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList4.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>";
	this.m__index=this.m__index+1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
	if(this.m__index>=0 && this.m__index<this.m__length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
		pop_err();
		return dbg_array(this.m__data,this.m__index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList4.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList4.prototype.p_Last=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<160>";
	this.m__index=this.m__length-1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<162>";
	if((this.m__length)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<162>";
		var t_=dbg_array(this.m__data,this.m__length-1)[dbg_index];
		pop_err();
		return t_;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<162>";
		pop_err();
		return null;
	}
}
c_iList4.prototype.p_Descend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<107>";
	this.m__index=this.m__index-1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<109>";
	if(this.m__index>=0 && this.m__index<this.m__length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<109>";
		pop_err();
		return dbg_array(this.m__data,this.m__index)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList4.prototype.p_Position8=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>";
	for(var t_i=0;t_i<this.m__length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
		if(dbg_array(this.m__data,t_i)[dbg_index]==t_value){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
			pop_err();
			return t_i;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>";
	pop_err();
	return -1;
}
c_iList4.prototype.p_RemoveFromIndex=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>";
	for(var t_i=t_index;t_i<this.m__length-1;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>";
		dbg_array(this.m__data,t_i)[dbg_index]=dbg_array(this.m__data,t_i+1)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>";
	this.m__length=this.m__length-1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>";
	this.m__index=this.m__index-1;
	pop_err();
}
c_iList4.prototype.p_Remove5=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>";
	var t_p=this.p_Position8(t_value);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
	if(t_p>=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
		this.p_RemoveFromIndex(t_p);
	}
	pop_err();
}
function bb_graphics_DrawRect(t_x,t_y,t_w,t_h){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<393>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<395>";
	bb_graphics_context.p_Validate();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<396>";
	bb_graphics_renderDevice.DrawRect(t_x,t_y,t_w,t_h);
	pop_err();
	return 0;
}
function c_iSystemGui(){
	c_iPlayfield.call(this);
}
c_iSystemGui.prototype=extend_class(c_iPlayfield);
c_iSystemGui.m__playfield=null;
c_iSystemGui.m_Playfield=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<107>";
	var t_=(c_iSystemGui.m__playfield);
	pop_err();
	return t_;
}
c_iSystemGui.m_GuiPage=function(t_page){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<114>";
	if((c_iSystemGui.m_Playfield())!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<114>";
		c_iSystemGui.m_Playfield().p_GuiPage2(t_page);
	}
	pop_err();
}
c_iSystemGui.prototype.p_Set2=function(){
	push_err();
	pop_err();
}
c_iSystemGui.prototype.p_SetAlpha=function(t_alpha){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<233>";
	bb_graphics_SetAlpha(t_alpha);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<234>";
	pop_err();
	return 0;
}
c_iSystemGui.prototype.p_SetColor=function(t_red,t_green,t_blue){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<241>";
	bb_graphics_SetColor(t_red,t_green,t_blue);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/systemgui.monkey<242>";
	pop_err();
	return 0;
}
function c_iTask(){
	c_iObject.call(this);
	this.m__visible=1;
	this.m__cold=true;
	this.m__started=false;
}
c_iTask.prototype=extend_class(c_iObject);
c_iTask.prototype.p_OnRender=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<80>";
	pop_err();
	return 0;
}
c_iTask.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<184>";
	this.p_OnRender();
	pop_err();
}
c_iTask.m_Render=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<192>";
	if((t_list)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<194>";
		if((t_list.p_Length())!=0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<196>";
			var t_t=t_list.p_First();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<197>";
			while((t_t)!=null){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<199>";
				if((dbg_object(t_t).m__enabled)!=0){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<201>";
					if(dbg_object(t_t).m__visible==1){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<202>";
						t_t.p_Render();
					}else{
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<204>";
						if(dbg_object(t_t).m__visible>1){
							err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<204>";
							dbg_object(t_t).m__visible=dbg_object(t_t).m__visible-1;
						}
					}
				}
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<209>";
				t_t=t_list.p_Ascend();
			}
		}
	}
	pop_err();
}
c_iTask.prototype.p_OnUpdate=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<87>";
	pop_err();
	return 0;
}
c_iTask.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<237>";
	this.p_OnUpdate();
	pop_err();
}
c_iTask.prototype.p_SystemStart=function(){
	push_err();
	pop_err();
}
c_iTask.prototype.p_OnColdStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<50>";
	pop_err();
	return 0;
}
c_iTask.prototype.p_OnStart=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<66>";
	pop_err();
	return 0;
}
c_iTask.m_Update=function(t_list){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<245>";
	if((t_list)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<247>";
		if((t_list.p_Length())!=0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<249>";
			var t_t=t_list.p_First();
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<251>";
			while((t_t)!=null){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<253>";
				if(dbg_object(t_t).m__enabled==1){
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<255>";
					if(dbg_object(t_t).m__cold==true || dbg_object(t_t).m__started==false){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<255>";
						t_t.p_SystemStart();
					}
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<257>";
					if(dbg_object(t_t).m__cold==true){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<258>";
						t_t.p_OnColdStart();
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<259>";
						dbg_object(t_t).m__cold=false;
					}
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<262>";
					if(dbg_object(t_t).m__started==false){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<263>";
						t_t.p_OnStart();
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<264>";
						dbg_object(t_t).m__started=true;
					}
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<267>";
					t_t.p_Update();
				}else{
					err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<271>";
					if(dbg_object(t_t).m__enabled>1){
						err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<271>";
						dbg_object(t_t).m__enabled=dbg_object(t_t).m__enabled-1;
					}
				}
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/task.monkey<275>";
				t_t=t_list.p_Ascend();
			}
		}
	}
	pop_err();
}
function c_iList5(){
	Object.call(this);
	this.m__length=0;
	this.m__index=0;
	this.m__data=[];
}
c_iList5.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList5.prototype.p_Length=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<170>";
	pop_err();
	return this.m__length;
}
c_iList5.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>";
	this.m__index=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
	if((this.m__length)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
		pop_err();
		return dbg_array(this.m__data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList5.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>";
	this.m__index=this.m__index+1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
	if(this.m__index>=0 && this.m__index<this.m__length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
		pop_err();
		return dbg_array(this.m__data,this.m__index)[dbg_index];
	}
	pop_err();
	return null;
}
var bb_globals_iTaskList=null;
function bb_math_Abs(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<46>";
	if(t_x>=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<46>";
		pop_err();
		return t_x;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<47>";
	var t_=-t_x;
	pop_err();
	return t_;
}
function bb_math_Abs2(t_x){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<73>";
	if(t_x>=0.0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<73>";
		pop_err();
		return t_x;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/math.monkey<74>";
	var t_=-t_x;
	pop_err();
	return t_;
}
var bb_globals_iMultiTouch=0;
function bb_input_TouchDown(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<84>";
	var t_=((bb_input_device.p_KeyDown(384+t_index))?1:0);
	pop_err();
	return t_;
}
function bb_functions_iTouchDowns(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<442>";
	var t_i=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<444>";
	for(t_i=0;t_i<bb_globals_iMultiTouch;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<445>";
		if((bb_input_TouchDown(t_i))!=0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<445>";
			t_i=t_i+1;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<448>";
	pop_err();
	return t_i;
}
function bb_input_TouchX(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<76>";
	var t_=bb_input_device.p_TouchX(t_index);
	pop_err();
	return t_;
}
function bb_input_TouchY(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<80>";
	var t_=bb_input_device.p_TouchY(t_index);
	pop_err();
	return t_;
}
function bb_functions_iGetTouchPlayfield(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<163>";
	if(dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_Length()==0 && ((dbg_object(bb_globals_iEnginePointer).m__playfieldPointer)!=null) && ((dbg_object(dbg_object(bb_globals_iEnginePointer).m__playfieldPointer).m__enabled)!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<163>";
		pop_err();
		return dbg_object(bb_globals_iEnginePointer).m__playfieldPointer;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<165>";
	if(dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_Length()==1 && ((dbg_object(dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_First()).m__enabled)!=0)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<167>";
		var t_p=dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_First();
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<169>";
		if(bb_input_TouchX(t_index)>=dbg_object(t_p).m__x && bb_input_TouchY(t_index)>=dbg_object(t_p).m__y && bb_input_TouchX(t_index)<=dbg_object(t_p).m__x+(dbg_object(t_p).m__vwidth)*dbg_object(t_p).m__scaleX && bb_input_TouchY(t_index)<=dbg_object(t_p).m__y+(dbg_object(t_p).m__vheight)*dbg_object(t_p).m__scaleY){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<169>";
			pop_err();
			return t_p;
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<173>";
		for(var t_i=dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_Length()-1;t_i>=0;t_i=t_i+-1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<175>";
			var t_p2=dbg_object(bb_globals_iEnginePointer).m__playfieldList.p_Get(t_i);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<177>";
			if(bb_input_TouchX(t_index)>=dbg_object(t_p2).m__x && bb_input_TouchY(t_index)>=dbg_object(t_p2).m__y && bb_input_TouchX(t_index)<=dbg_object(t_p2).m__x+(dbg_object(t_p2).m__vwidth)*dbg_object(t_p2).m__scaleX && bb_input_TouchY(t_index)<=dbg_object(t_p2).m__y+(dbg_object(t_p2).m__vheight)*dbg_object(t_p2).m__scaleY){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<177>";
				pop_err();
				return t_p2;
			}
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/functions.monkey<183>";
	pop_err();
	return null;
}
function bb_commoncode_iPointInsideRectange(t_x,t_y,t_width,t_height,t_pointX,t_pointY,t_rotation,t_handleX,t_handleY){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<92>";
	var t_c=Math.cos((t_rotation)*D2R);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<93>";
	var t_s=Math.sin((t_rotation)*D2R);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<95>";
	var t_rotatedX=t_x+t_c*(t_pointX-t_x)-t_s*(t_pointY-t_y)+t_handleX;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<96>";
	var t_rotatedY=t_y+t_s*(t_pointX-t_x)+t_c*(t_pointY-t_y)+t_handleY;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<98>";
	var t_leftX=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<99>";
	var t_rightX=t_x+t_width;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<100>";
	var t_topY=t_y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<101>";
	var t_bottomY=t_y+t_height;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/commoncode.monkey<103>";
	var t_=t_leftX<=t_rotatedX && t_rotatedX<=t_rightX && t_topY<=t_rotatedY && t_rotatedY<=t_bottomY;
	pop_err();
	return t_;
}
function c_iCollision(){
	Object.call(this);
}
c_iCollision.m_Rectangles=function(t_x1,t_y1,t_width1,t_height1,t_x2,t_y2,t_width2,t_height2,t_centerRect){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<137>";
	if((t_centerRect)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<138>";
		t_x1=t_x1-((t_width1/2)|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<139>";
		t_y1=t_y1-((t_height1/2)|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<140>";
		t_x2=t_x2-((t_width2/2)|0);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<141>";
		t_y2=t_y2-((t_height2/2)|0);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<144>";
	if(t_x1+(t_width1)<=t_x2){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<144>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<145>";
	if(t_y1+(t_height1)<=t_y2){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<145>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<147>";
	if(t_x1>=t_x2+(t_width2)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<147>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<148>";
	if(t_y1>=t_y2+(t_height2)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<148>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/collision.monkey<150>";
	pop_err();
	return true;
}
function c_iList6(){
	Object.call(this);
	this.m__length=0;
	this.m__data=[];
	this.m__index=0;
}
c_iList6.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<19>";
	pop_err();
	return this;
}
c_iList6.prototype.p_AddLast4=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<55>";
	if(this.m__length==this.m__data.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<56>";
		this.m__data=resize_object_array(this.m__data,this.m__length*2+10);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<58>";
	dbg_array(this.m__data,this.m__length)[dbg_index]=t_value;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<59>";
	this.m__length=this.m__length+1;
	pop_err();
}
c_iList6.prototype.p_Length=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<170>";
	pop_err();
	return this.m__length;
}
c_iList6.prototype.p_First=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<118>";
	this.m__index=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
	if((this.m__length)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<120>";
		pop_err();
		return dbg_array(this.m__data,0)[dbg_index];
	}
	pop_err();
	return null;
}
c_iList6.prototype.p_Position9=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<184>";
	for(var t_i=0;t_i<this.m__length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
		if(dbg_array(this.m__data,t_i)[dbg_index]==t_value){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<185>";
			pop_err();
			return t_i;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<187>";
	pop_err();
	return -1;
}
c_iList6.prototype.p_RemoveFromIndex=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<210>";
	for(var t_i=t_index;t_i<this.m__length-1;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<211>";
		dbg_array(this.m__data,t_i)[dbg_index]=dbg_array(this.m__data,t_i+1)[dbg_index];
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<214>";
	this.m__length=this.m__length-1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<215>";
	this.m__index=this.m__index-1;
	pop_err();
}
c_iList6.prototype.p_Remove6=function(t_value){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<197>";
	var t_p=this.p_Position9(t_value);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
	if(t_p>=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<199>";
		this.p_RemoveFromIndex(t_p);
	}
	pop_err();
}
c_iList6.prototype.p_Ascend=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<67>";
	this.m__index=this.m__index+1;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
	if(this.m__index>=0 && this.m__index<this.m__length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/list.monkey<69>";
		pop_err();
		return dbg_array(this.m__data,this.m__index)[dbg_index];
	}
	pop_err();
	return null;
}
function bb_graphics_DrawImage(t_image,t_x,t_y,t_frame){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<452>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<453>";
	if(t_frame<0 || t_frame>=dbg_object(t_image).m_frames.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<453>";
		error("Invalid image frame");
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<456>";
	var t_f=dbg_array(dbg_object(t_image).m_frames,t_frame)[dbg_index];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<458>";
	bb_graphics_context.p_Validate();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<460>";
	if((dbg_object(t_image).m_flags&65536)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<461>";
		bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).m_surface,t_x-dbg_object(t_image).m_tx,t_y-dbg_object(t_image).m_ty);
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<463>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).m_surface,t_x-dbg_object(t_image).m_tx,t_y-dbg_object(t_image).m_ty,dbg_object(t_f).m_x,dbg_object(t_f).m_y,dbg_object(t_image).m_width,dbg_object(t_image).m_height);
	}
	pop_err();
	return 0;
}
function bb_graphics_PushMatrix(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<333>";
	var t_sp=dbg_object(bb_graphics_context).m_matrixSp;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<334>";
	if(t_sp==dbg_object(bb_graphics_context).m_matrixStack.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<334>";
		dbg_object(bb_graphics_context).m_matrixStack=resize_number_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp*2);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<335>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+0)[dbg_index]=dbg_object(bb_graphics_context).m_ix;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<336>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+1)[dbg_index]=dbg_object(bb_graphics_context).m_iy;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<337>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+2)[dbg_index]=dbg_object(bb_graphics_context).m_jx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<338>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+3)[dbg_index]=dbg_object(bb_graphics_context).m_jy;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<339>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+4)[dbg_index]=dbg_object(bb_graphics_context).m_tx;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<340>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+5)[dbg_index]=dbg_object(bb_graphics_context).m_ty;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<341>";
	dbg_object(bb_graphics_context).m_matrixSp=t_sp+6;
	pop_err();
	return 0;
}
function bb_graphics_PopMatrix(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<345>";
	var t_sp=dbg_object(bb_graphics_context).m_matrixSp-6;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<346>";
	bb_graphics_SetMatrix(dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+0)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+1)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+2)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+3)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+4)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+5)[dbg_index]);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<347>";
	dbg_object(bb_graphics_context).m_matrixSp=t_sp;
	pop_err();
	return 0;
}
function bb_graphics_DrawImage2(t_image,t_x,t_y,t_rotation,t_scaleX,t_scaleY,t_frame){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<470>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<471>";
	if(t_frame<0 || t_frame>=dbg_object(t_image).m_frames.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<471>";
		error("Invalid image frame");
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<474>";
	var t_f=dbg_array(dbg_object(t_image).m_frames,t_frame)[dbg_index];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<476>";
	bb_graphics_PushMatrix();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<478>";
	bb_graphics_Translate(t_x,t_y);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<479>";
	bb_graphics_Rotate(t_rotation);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<480>";
	bb_graphics_Scale(t_scaleX,t_scaleY);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<482>";
	bb_graphics_Translate(-dbg_object(t_image).m_tx,-dbg_object(t_image).m_ty);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<484>";
	bb_graphics_context.p_Validate();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<486>";
	if((dbg_object(t_image).m_flags&65536)!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<487>";
		bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).m_surface,0.0,0.0);
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<489>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).m_surface,0.0,0.0,dbg_object(t_f).m_x,dbg_object(t_f).m_y,dbg_object(t_image).m_width,dbg_object(t_image).m_height);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<492>";
	bb_graphics_PopMatrix();
	pop_err();
	return 0;
}
function bb_graphics_DrawText(t_text,t_x,t_y,t_xalign,t_yalign){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<577>";
	bb_graphics_DebugRenderDevice();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<579>";
	if(!((dbg_object(bb_graphics_context).m_font)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<579>";
		pop_err();
		return 0;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<581>";
	var t_w=dbg_object(bb_graphics_context).m_font.p_Width();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<582>";
	var t_h=dbg_object(bb_graphics_context).m_font.p_Height();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<584>";
	t_x-=Math.floor((t_w*t_text.length)*t_xalign);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<585>";
	t_y-=Math.floor((t_h)*t_yalign);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<587>";
	for(var t_i=0;t_i<t_text.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<588>";
		var t_ch=dbg_charCodeAt(t_text,t_i)-dbg_object(bb_graphics_context).m_firstChar;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<589>";
		if(t_ch>=0 && t_ch<dbg_object(bb_graphics_context).m_font.p_Frames()){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/graphics.monkey<590>";
			bb_graphics_DrawImage(dbg_object(bb_graphics_context).m_font,t_x+(t_i*t_w),t_y,t_ch);
		}
	}
	pop_err();
	return 0;
}
function bb_input_KeyHit(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<44>";
	var t_=bb_input_device.p_KeyHit(t_key);
	pop_err();
	return t_;
}
function bb_gfx_iLoadSprite(t_path,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<390>";
	var t_=bb_graphics_LoadImage(t_path,t_frameCount,1);
	pop_err();
	return t_;
}
function bb_gfx_iLoadSprite2(t_path,t_frameWidth,t_frameHeight,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<399>";
	var t_=bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,1);
	pop_err();
	return t_;
}
function bb_strings_iStripExt(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<261>";
	var t_i=t_path.lastIndexOf(".");
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<262>";
	if(t_i!=-1 && t_path.indexOf("/",t_i+1)==-1 && t_path.indexOf("\\",t_i+1)==-1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<262>";
		var t_=t_path.slice(0,t_i);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<264>";
	pop_err();
	return t_path;
}
function bb_strings_iExtractExt(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<319>";
	var t_i=t_path.lastIndexOf(".");
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<320>";
	if(t_i!=-1 && t_path.indexOf("/",t_i+1)==-1 && t_path.indexOf("\\",t_i+1)==-1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<320>";
		var t_=t_path.slice(t_i+1);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/strings.monkey<321>";
	pop_err();
	return "";
}
function bb_gfx_iLoadImage(t_start,t_count,t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<324>";
	var t_i=0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<326>";
	var t_image=new_object_array(t_count);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<328>";
	var t_file=bb_strings_iStripExt(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<329>";
	var t_extension="."+bb_strings_iExtractExt(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<331>";
	for(var t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<332>";
		if(t_c<10){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<332>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"000"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<333>";
		if(t_c>9 && t_c<100){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<333>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"00"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<334>";
		if(t_c>99 && t_c<1000){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<334>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"0"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<335>";
		t_i=t_i+1;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<338>";
	pop_err();
	return t_image;
}
function bb_gfx_iLoadImage2(t_image,t_start,t_count,t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<344>";
	var t_i=t_image.length;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<346>";
	t_image=resize_object_array(t_image,t_image.length+t_count);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<348>";
	var t_file=bb_strings_iStripExt(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<349>";
	var t_extension="."+bb_strings_iExtractExt(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<351>";
	for(var t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<352>";
		if(t_c<10){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<352>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"000"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<353>";
		if(t_c>9 && t_c<100){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<353>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"00"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<354>";
		if(t_c>99 && t_c<1000){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<354>";
			dbg_array(t_image,t_i)[dbg_index]=bb_graphics_LoadImage(t_file+"0"+String(t_c)+t_extension,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<355>";
		t_i=t_i+1;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<358>";
	pop_err();
	return t_image;
}
function bb_gfx_iLoadImage3(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<369>";
	var t_i=new_object_array(t_count);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<371>";
	var t_f=bb_strings_iStripExt(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<372>";
	var t_e="."+bb_strings_iExtractExt(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<374>";
	for(var t_c=t_start;t_c<t_start+t_count;t_c=t_c+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<375>";
		if(t_c<10){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<375>";
			dbg_array(t_i,t_c)[dbg_index]=bb_graphics_LoadImage2(t_f+"000"+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<376>";
		if(t_c>9 && t_c<100){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<376>";
			dbg_array(t_i,t_c)[dbg_index]=bb_graphics_LoadImage2(t_f+"00"+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<377>";
		if(t_c>99 && t_c<1000){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<377>";
			dbg_array(t_i,t_c)[dbg_index]=bb_graphics_LoadImage2(t_f+"0"+String(t_c)+t_e,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<380>";
	pop_err();
	return t_i;
}
function bb_gfx_iLoadSprite3(t_start,t_count,t_path,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<408>";
	var t_=bb_gfx_iLoadImage(t_start,t_count,t_path,t_frameCount,1);
	pop_err();
	return t_;
}
function bb_gfx_iLoadSprite4(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/gfx.monkey<417>";
	var t_=bb_gfx_iLoadImage3(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,1);
	pop_err();
	return t_;
}
function c_iLayerSprite(){
	c_iLayerObject.call(this);
	this.m__imagePointer=new_object_array(1);
	this.m__imageSignature=new_object_array(1);
	this.m__ghostImagePointer=new_object_array(1);
	this.m__frame=.0;
	this.m__imageIndex=.0;
	this.m__frameOffset=1.0;
	this.m__animationMode=1;
	this.m__imagePath="";
	this.implments={c_IMover:1};
}
c_iLayerSprite.prototype=extend_class(c_iLayerObject);
c_iLayerSprite.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<11>";
	c_iLayerObject.m_new.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<11>";
	pop_err();
	return this;
}
c_iLayerSprite.prototype.p_ImagePointer=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<436>";
	pop_err();
	return this.m__imagePointer;
}
c_iLayerSprite.prototype.p_ImagePointer2=function(t_image){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<444>";
	if(t_image!=dbg_array(this.m__imageSignature,0)[dbg_index]){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<446>";
		dbg_array(this.m__imagePointer,0)[dbg_index]=t_image;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<447>";
		dbg_array(this.m__ghostImagePointer,0)[dbg_index]=t_image;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<448>";
		dbg_array(this.m__imageSignature,0)[dbg_index]=t_image;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<450>";
		dbg_array(this.m__imagePointer,0)[dbg_index].p_SetHandle((t_image.p_Width())*.5,(t_image.p_Height())*.5);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<452>";
		this.m__frame=0.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<453>";
		this.m__imageIndex=0.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<455>";
		this.m__frameOffset=1.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<456>";
		this.m__animationMode=1;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<458>";
		if(this.m__imagePointer.length>1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<458>";
			this.m__imagePointer=resize_object_array(this.m__imagePointer,1);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<459>";
		if(this.m__ghostImagePointer.length>1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<459>";
			this.m__ghostImagePointer=resize_object_array(this.m__ghostImagePointer,1);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<460>";
		if(this.m__imageSignature.length>1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<460>";
			this.m__imageSignature=resize_object_array(this.m__imageSignature,1);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<462>";
		this.m__imagePath="";
	}
	pop_err();
}
c_iLayerSprite.prototype.p_Compare2=function(t_image1,t_image2){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<229>";
	if(t_image1.length!=t_image2.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<229>";
		pop_err();
		return false;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<231>";
	for(var t_i=0;t_i<t_image1.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<232>";
		if(dbg_array(t_image1,t_i)[dbg_index]!=dbg_array(t_image2,t_i)[dbg_index]){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<232>";
			pop_err();
			return false;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<235>";
	pop_err();
	return true;
}
c_iLayerSprite.prototype.p_ImagePointer3=function(t_image){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<473>";
	if(this.p_Compare2(t_image,this.m__imageSignature)==false){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<475>";
		this.m__imagePointer=resize_object_array(this.m__imagePointer,t_image.length);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<476>";
		for(var t_i=0;t_i<t_image.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<477>";
			dbg_array(this.m__imagePointer,t_i)[dbg_index]=dbg_array(t_image,t_i)[dbg_index];
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<480>";
		this.m__ghostImagePointer=resize_object_array(this.m__ghostImagePointer,t_image.length);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<481>";
		for(var t_i2=0;t_i2<t_image.length;t_i2=t_i2+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<482>";
			dbg_array(this.m__ghostImagePointer,t_i2)[dbg_index]=dbg_array(t_image,t_i2)[dbg_index];
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<485>";
		this.m__imageSignature=resize_object_array(this.m__imageSignature,t_image.length);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<486>";
		for(var t_i3=0;t_i3<t_image.length;t_i3=t_i3+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<487>";
			dbg_array(this.m__imageSignature,t_i3)[dbg_index]=dbg_array(t_image,t_i3)[dbg_index];
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<490>";
		for(var t_i4=0;t_i4<this.m__imagePointer.length;t_i4=t_i4+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<491>";
			dbg_array(this.m__imagePointer,t_i4)[dbg_index].p_SetHandle((dbg_array(this.m__imagePointer,t_i4)[dbg_index].p_Width())*.5,(dbg_array(this.m__imagePointer,t_i4)[dbg_index].p_Height())*.5);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<494>";
		this.m__frame=0.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<495>";
		this.m__imageIndex=0.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<497>";
		this.m__frameOffset=1.0;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<498>";
		this.m__animationMode=1;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<500>";
		this.m__imagePath="";
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImagePointer4=function(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<575>";
	if(t_path!=this.m__imagePath){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<577>";
		var t_image=bb_contentmanager_iContent.p_GetImage4(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,c_Image.m_DefaultFlags);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<579>";
		this.p_ImagePointer3(t_image);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<580>";
		this.m__imagePath=t_path;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImagePointer5=function(t_start,t_count,t_path,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<555>";
	if(t_path!=this.m__imagePath){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<557>";
		var t_image=bb_contentmanager_iContent.p_GetImage3(t_start,t_count,t_path,t_frameCount,c_Image.m_DefaultFlags);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<559>";
		this.p_ImagePointer3(t_image);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<560>";
		this.m__imagePath=t_path;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImagePointer6=function(t_path,t_frameWidth,t_frameHeight,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<535>";
	if(t_path!=this.m__imagePath){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<537>";
		var t_image=bb_contentmanager_iContent.p_GetImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,c_Image.m_DefaultFlags);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<539>";
		this.p_ImagePointer2(t_image);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<540>";
		this.m__imagePath=t_path;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImagePointer7=function(t_path,t_frameCount){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<515>";
	if(t_path!=this.m__imagePath){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<517>";
		var t_image=bb_contentmanager_iContent.p_GetImage(t_path,t_frameCount,c_Image.m_DefaultFlags);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<519>";
		this.p_ImagePointer2(t_image);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<520>";
		this.m__imagePath=t_path;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_SetImage=function(t_image){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<714>";
	this.p_ImagePointer2(t_image);
	pop_err();
}
c_iLayerSprite.prototype.p_SetImage2=function(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<722>";
	if(t_path!=this.m__imagePath){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<724>";
		var t_o=bb_contentmanager_iContent.p_Get2(t_path);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<725>";
		if((t_o)!=null){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<727>";
			if((dbg_object(t_o).m__imagePointer)!=null){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<727>";
				this.p_ImagePointer2(dbg_object(t_o).m__imagePointer);
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<728>";
			if((dbg_object(t_o).m__imagePointers.length)!=0){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<728>";
				this.p_ImagePointer3(dbg_object(t_o).m__imagePointers);
			}
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<730>";
			this.m__imagePath=t_path;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<732>";
			this.m__animationMode=1;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<733>";
			this.m__frame=0.0;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<734>";
			this.m__frameOffset=.025;
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<735>";
			this.m__imageIndex=0.0;
		}
	}
	pop_err();
}
c_iLayerSprite.prototype.p_AnimationLoop=function(t_frameOffset,t_path){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<55>";
	if(t_frameOffset==0.0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<55>";
		t_frameOffset=this.m__frameOffset;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<57>";
	if((t_path).length!=0){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<57>";
		this.p_SetImage2(t_path);
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<59>";
	var t_finished=false;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<61>";
	if(this.m__imagePointer.length>1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<63>";
		this.m__imageIndex=this.m__imageIndex+t_frameOffset*(this.m__animationMode);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<65>";
		if(this.m__imageIndex<0.0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<66>";
			this.m__imageIndex=this.m__imageIndex+(this.m__imagePointer.length);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<67>";
			t_finished=true;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<70>";
		if(this.m__imageIndex>=(this.m__imagePointer.length)){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<71>";
			this.m__imageIndex=this.m__imageIndex-(this.m__imagePointer.length);
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<72>";
			t_finished=true;
		}
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<77>";
		this.m__frame=this.m__frame+t_frameOffset*(this.m__animationMode);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<79>";
		if(this.m__frame<0.0){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<80>";
			this.m__frame=this.m__frame+(dbg_array(this.m__imagePointer,((this.m__imageIndex)|0))[dbg_index].p_Frames());
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<81>";
			t_finished=true;
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<84>";
		if(this.m__frame>=(dbg_array(this.m__imagePointer,((this.m__imageIndex)|0))[dbg_index].p_Frames())){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<85>";
			this.m__frame=this.m__frame-(dbg_array(this.m__imagePointer,((this.m__imageIndex)|0))[dbg_index].p_Frames());
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<86>";
			t_finished=true;
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<91>";
	pop_err();
	return t_finished;
}
c_iLayerSprite.prototype.p_Frame=function(t_frame){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<297>";
	if(this.m__imagePointer.length>1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<298>";
		this.m__imageIndex=t_frame;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<300>";
		this.m__frame=t_frame;
	}
	pop_err();
}
c_iLayerSprite.prototype.p_ImageIndex=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<335>";
	var t_=this.m__imageIndex % (this.m__imagePointer.length);
	pop_err();
	return t_;
}
c_iLayerSprite.prototype.p_ImageIndex2=function(t_index){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<342>";
	this.m__imageIndex=t_index;
	pop_err();
}
c_iLayerSprite.prototype.p_ImageFrame=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<421>";
	var t_=this.m__frame % (dbg_array(this.m__imagePointer,((this.p_ImageIndex())|0))[dbg_index].p_Frames());
	pop_err();
	return t_;
}
c_iLayerSprite.prototype.p_Frame2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<310>";
	if(this.m__imagePointer.length>1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<311>";
		var t_=this.p_ImageIndex();
		pop_err();
		return t_;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<313>";
		var t_2=this.p_ImageFrame();
		pop_err();
		return t_2;
	}
}
c_iLayerSprite.prototype.p_Destroy=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<280>";
	c_iLayerObject.prototype.p_Destroy.call(this);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<282>";
	this.m__imagePointer=[];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<283>";
	this.m__imageSignature=[];
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<284>";
	this.m__ghostImagePointer=[];
	pop_err();
}
c_iLayerSprite.prototype.p_ImageLoaded=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<429>";
	if((dbg_array(this.m__imagePointer,((this.p_ImageIndex())|0))[dbg_index])!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<429>";
		pop_err();
		return true;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<429>";
		pop_err();
		return false;
	}
}
c_iLayerSprite.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<416>";
	if(this.p_ImageLoaded()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<416>";
		var t_=bb_math_Abs2((dbg_array(this.m__imagePointer,((this.p_ImageIndex())|0))[dbg_index].p_Height())*this.m__scaleY);
		pop_err();
		return t_;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<416>";
		pop_err();
		return 0.0;
	}
}
c_iLayerSprite.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<659>";
	if(!this.p_ImageLoaded()){
		pop_err();
		return;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<661>";
	if(this.m__ghost){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<664>";
		if(bb_graphics_GetBlend()!=this.m__ghostBlend){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<664>";
			bb_graphics_SetBlend(this.m__ghostBlend);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<668>";
		this.p_SetAlpha(this.m__ghostAlpha);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<672>";
		this.p_SetColor(this.m__ghostRed,this.m__ghostGreen,this.m__ghostBlue);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<675>";
		bb_graphics_DrawImage2(dbg_array(this.m__ghostImagePointer,((this.p_ImageIndex())|0))[dbg_index],this.p_RenderX(),this.p_RenderY(),this.p_Rotation()*(bb_globals_iRotation),this.p_ScaleX(),this.p_ScaleY(),((this.p_ImageFrame())|0));
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<677>";
		this.m__ghost=false;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<682>";
		if(bb_graphics_GetBlend()!=this.m__blend){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<682>";
			bb_graphics_SetBlend(this.m__blend);
		}
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<686>";
		this.p_SetAlpha(this.m__alpha);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<690>";
		this.p_SetColor(this.m__red,this.m__green,this.m__blue);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<693>";
		bb_graphics_DrawImage2(dbg_array(this.m__imagePointer,((this.p_ImageIndex())|0))[dbg_index],this.p_RenderX(),this.p_RenderY(),this.p_Rotation()*(bb_globals_iRotation),this.p_ScaleX(),this.p_ScaleY(),((this.p_ImageFrame())|0));
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<697>";
	this.p_RenderDebugInfo();
	pop_err();
}
c_iLayerSprite.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<747>";
	this.p_ClearDebugInfo();
	pop_err();
}
c_iLayerSprite.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<754>";
	if(this.p_ImageLoaded()){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<754>";
		var t_=bb_math_Abs2((dbg_array(this.m__imagePointer,((this.p_ImageIndex())|0))[dbg_index].p_Width())*this.m__scaleX);
		pop_err();
		return t_;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layersprite.monkey<754>";
		pop_err();
		return 0.0;
	}
}
function c_iLayerGroup(){
	c_iLayerObject.call(this);
	this.m__objectList=c_iList3.m_new.call(new c_iList3);
	this.implments={c_IMover:1};
}
c_iLayerGroup.prototype=extend_class(c_iLayerObject);
c_iLayerGroup.prototype.p_AddItemLast=function(t_layerObject){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<45>";
	this.m__objectList.p_AddLast3(t_layerObject);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<47>";
	t_layerObject.p_Layer2(this.p_Layer());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<49>";
	t_layerObject.p_Parent2(this);
	pop_err();
}
c_iLayerGroup.prototype.p_Height=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<122>";
	var t_=(this.m__height)*this.m__scaleY;
	pop_err();
	return t_;
}
c_iLayerGroup.prototype.p_Render=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<177>";
	this.p_RenderList(this.m__objectList);
	pop_err();
}
c_iLayerGroup.prototype.p_Update=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<245>";
	this.p_UpdateList(this.m__objectList);
	pop_err();
}
c_iLayerGroup.prototype.p_Width=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<269>";
	var t_=(this.m__width)*this.m__scaleX;
	pop_err();
	return t_;
}
c_iLayerGroup.prototype.p_UpdateWorldXY=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<253>";
	this.m__worldHeight=bb_math_Abs2(this.p_Height());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<254>";
	this.m__worldScaleX=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<255>";
	this.m__worldScaleY=this.p_ScaleX();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<256>";
	this.m__worldRotation=this.p_Rotation();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<257>";
	this.m__worldWidth=bb_math_Abs2(this.p_Width());
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<258>";
	this.m__worldX=this.m__x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<259>";
	this.m__worldY=this.m__y;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/engine/components/layergroup.monkey<261>";
	this.p_UpdateWorldXYList(this.m__objectList);
	pop_err();
}
function c_iContentManager(){
	Object.call(this);
	this.m__data=new_object_array(128);
	this.m__length=0;
	this.m__cache=0;
}
c_iContentManager.prototype.p_SystemInit=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<525>";
	for(var t_i=0;t_i<this.m__data.length;t_i=t_i+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<526>";
		dbg_array(this.m__data,t_i)[dbg_index]=c_iContentObject.m_new.call(new c_iContentObject);
	}
	pop_err();
}
c_iContentManager.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<476>";
	this.p_SystemInit();
	pop_err();
	return this;
}
c_iContentManager.prototype.p_Expand=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<380>";
	if(this.m__length==this.m__data.length){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<382>";
		this.m__data=resize_object_array(this.m__data,this.m__length*2+10);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<384>";
		for(var t_i=0;t_i<this.m__data.length;t_i=t_i+1){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<385>";
			if(!((dbg_array(this.m__data,t_i)[dbg_index])!=null)){
				err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<385>";
				dbg_array(this.m__data,t_i)[dbg_index]=c_iContentObject.m_new.call(new c_iContentObject);
			}
		}
	}
	pop_err();
}
c_iContentManager.prototype.p_Get2=function(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<70>";
	for(this.m__cache=0;this.m__cache<this.m__length;this.m__cache=this.m__cache+1){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<71>";
		if(dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__path==t_path){
			err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<71>";
			pop_err();
			return dbg_array(this.m__data,this.m__cache)[dbg_index];
		}
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<73>";
	pop_err();
	return null;
}
c_iContentManager.prototype.p_GetImage=function(t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<160>";
	if(this.m__length>0 && ((dbg_array(this.m__data,this.m__cache)[dbg_index])!=null) && dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__path==t_path){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<160>";
		pop_err();
		return dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__imagePointer;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<162>";
	this.p_Expand();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<164>";
	var t_o=this.p_Get2(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<166>";
	if(((t_o)!=null) && ((dbg_object(t_o).m__imagePointer)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<168>";
		pop_err();
		return dbg_object(t_o).m__imagePointer;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<172>";
		t_o=dbg_array(this.m__data,this.m__length)[dbg_index];
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<174>";
		this.m__length=this.m__length+1;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<176>";
		dbg_object(t_o).m__imagePointer=bb_graphics_LoadImage(t_path,t_frameCount,t_flags);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<178>";
		dbg_object(t_o).m__path=t_path;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<180>";
		pop_err();
		return dbg_object(t_o).m__imagePointer;
	}
}
c_iContentManager.prototype.p_GetImage2=function(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<205>";
	if(this.m__length>0 && ((dbg_array(this.m__data,this.m__cache)[dbg_index])!=null) && dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__path==t_path){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<205>";
		pop_err();
		return dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__imagePointer;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<207>";
	this.p_Expand();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<209>";
	var t_o=this.p_Get2(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<211>";
	if(((t_o)!=null) && ((dbg_object(t_o).m__imagePointer)!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<213>";
		pop_err();
		return dbg_object(t_o).m__imagePointer;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<217>";
		t_o=dbg_array(this.m__data,this.m__length)[dbg_index];
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<219>";
		this.m__length=this.m__length+1;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<221>";
		dbg_object(t_o).m__imagePointer=bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<223>";
		dbg_object(t_o).m__path=t_path;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<225>";
		pop_err();
		return dbg_object(t_o).m__imagePointer;
	}
}
c_iContentManager.prototype.p_GetImage3=function(t_start,t_count,t_path,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<242>";
	if(this.m__length>0 && ((dbg_array(this.m__data,this.m__cache)[dbg_index])!=null) && dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__path==t_path){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<242>";
		pop_err();
		return dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__imagePointers;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<244>";
	this.p_Expand();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<246>";
	var t_o=this.p_Get2(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<248>";
	if(((t_o)!=null) && ((dbg_array(dbg_object(t_o).m__imagePointers,0)[dbg_index])!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<250>";
		pop_err();
		return dbg_object(t_o).m__imagePointers;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<254>";
		t_o=dbg_array(this.m__data,this.m__length)[dbg_index];
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<256>";
		this.m__length=this.m__length+1;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<258>";
		dbg_object(t_o).m__imagePointers=bb_gfx_iLoadImage(t_start,t_count,t_path,t_frameCount,t_flags);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<260>";
		dbg_object(t_o).m__path=t_path;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<262>";
		pop_err();
		return dbg_object(t_o).m__imagePointers;
	}
}
c_iContentManager.prototype.p_GetImage4=function(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<279>";
	if(this.m__length>0 && ((dbg_array(this.m__data,this.m__cache)[dbg_index])!=null) && dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__path==t_path){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<279>";
		pop_err();
		return dbg_object(dbg_array(this.m__data,this.m__cache)[dbg_index]).m__imagePointers;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<281>";
	this.p_Expand();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<283>";
	var t_o=this.p_Get2(t_path);
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<285>";
	if(((t_o)!=null) && ((dbg_array(dbg_object(t_o).m__imagePointers,0)[dbg_index])!=null)){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<287>";
		pop_err();
		return dbg_object(t_o).m__imagePointers;
	}else{
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<291>";
		t_o=dbg_array(this.m__data,this.m__length)[dbg_index];
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<293>";
		this.m__length=this.m__length+1;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<295>";
		dbg_object(t_o).m__imagePointers=bb_gfx_iLoadImage3(t_start,t_count,t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<297>";
		dbg_object(t_o).m__path=t_path;
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<299>";
		pop_err();
		return dbg_object(t_o).m__imagePointers;
	}
}
function c_iContentObject(){
	Object.call(this);
	this.m__path="";
	this.m__imagePointer=null;
	this.m__imagePointers=[];
}
c_iContentObject.m_new=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/framework/contentmanager.monkey<546>";
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
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<21>";
	dbg_object(this).m_image=t_i;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<22>";
	dbg_object(this).m_x=t_x;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<23>";
	dbg_object(this).m_y=t_y;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<24>";
	dbg_object(this).m_xVel=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<25>";
	dbg_object(this).m_yVel=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<26>";
	dbg_object(this).m_downAnim=c_Animation.m_new.call(new c_Animation,t_i,4,69,102,200);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<27>";
	dbg_object(this).m_currentAnimation=dbg_object(this).m_downAnim;
	pop_err();
	return this;
}
c_Player.m_new2=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<7>";
	pop_err();
	return this;
}
c_Player.prototype.p_HandleControls=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<52>";
	if((bb_input_KeyDown(37))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<53>";
		this.m_x=this.m_x-4;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<55>";
	if((bb_input_KeyDown(39))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<56>";
		this.m_x=this.m_x+4;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<58>";
	if((bb_input_KeyDown(40))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<59>";
		this.m_y=this.m_y+4;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<61>";
	if((bb_input_KeyDown(38))!=0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<62>";
		this.m_y=this.m_y-4;
	}
	pop_err();
	return 0;
}
c_Player.prototype.p_Update=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<35>";
	this.p_HandleControls();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/player.monkey<36>";
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
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<20>";
	dbg_object(this).m_img=t_i;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<21>";
	dbg_object(this).m_frames=t_f;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<22>";
	dbg_object(this).m_width=t_w;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<23>";
	dbg_object(this).m_height=t_h;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<24>";
	dbg_object(this).m_frameTime=t_ft;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<26>";
	dbg_object(this).m_elapsed=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<27>";
	dbg_object(this).m_frame=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<28>";
	dbg_object(this).m_lastTime=0;
	pop_err();
	return this;
}
c_Animation.m_new2=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<6>";
	pop_err();
	return this;
}
c_Animation.prototype.p_getFrame=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<35>";
	this.m_elapsed=this.m_elapsed+(bb_app_Millisecs()-this.m_lastTime);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<36>";
	this.m_lastTime=bb_app_Millisecs();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<38>";
	var t_toReturn=((Math.floor((this.m_elapsed/this.m_frameTime)|0))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<40>";
	if(t_toReturn>this.m_frames-1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<41>";
		t_toReturn=0;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<42>";
		this.m_elapsed=0;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/animation.monkey<45>";
	pop_err();
	return t_toReturn;
}
function c_Sound(){
	Object.call(this);
	this.m_sample=null;
}
c_Sound.m_new=function(t_sample){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<32>";
	dbg_object(this).m_sample=t_sample;
	pop_err();
	return this;
}
c_Sound.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<29>";
	pop_err();
	return this;
}
function bb_audio_LoadSound(t_path){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<47>";
	var t_sample=bb_audio_device.LoadSample(bb_data_FixDataPath(t_path));
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<48>";
	if((t_sample)!=null){
		err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<48>";
		var t_=c_Sound.m_new.call(new c_Sound,t_sample);
		pop_err();
		return t_;
	}
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/audio.monkey<49>";
	pop_err();
	return null;
}
function c_Level(){
	Object.call(this);
	this.m_layout=[];
	this.m_generated=false;
	this.m_xCoord=0;
	this.m_yCoord=0;
	this.m_width=0;
	this.m_height=0;
}
c_Level.prototype.p_Draw=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<223>";
	for(var t_i=0;t_i<dbg_object(this).m_layout.length;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<224>";
		for(var t_j=0;t_j<dbg_array(dbg_object(this).m_layout,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<225>";
			if(dbg_array(dbg_array(dbg_object(this).m_layout,t_i)[dbg_index],t_j)[dbg_index]=="X"){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<226>";
				bb_graphics_SetColor(0.0,0.0,0.0);
			}else{
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<228>";
				bb_graphics_SetColor(255.0,255.0,255.0);
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<230>";
			bb_graphics_DrawRect((t_i*40),(t_j*40),40.0,40.0);
		}
	}
	pop_err();
	return 0;
}
c_Level.prototype.p_setArray=function(t_i,t_j){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<59>";
	var t_result=new_array_array(t_i);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<61>";
	for(var t_index=0;t_index<t_i;t_index=t_index+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<62>";
		dbg_array(t_result,t_index)[dbg_index]=new_string_array(t_j);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<65>";
	pop_err();
	return t_result;
}
c_Level.prototype.p_randomlyAssignCells=function(t_design){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<72>";
	bb_random_Seed=bb_app_Millisecs();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<73>";
	var t_rand=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<75>";
	for(var t_i=0;t_i<t_design.length;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<76>";
		for(var t_j=0;t_j<dbg_array(t_design,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<77>";
			t_rand=((bb_random_Rnd2(0.0,100.0))|0);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<78>";
			if(t_rand<45){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<79>";
				dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]="X";
			}else{
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<81>";
				dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]="O";
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<83>";
			if(t_i==0 || t_j==0 || t_i==t_design.length-1 || t_j==dbg_array(t_design,0)[dbg_index].length-1){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<84>";
				dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]="X";
			}
		}
	}
	pop_err();
	return 0;
}
c_Level.prototype.p_checkWalls=function(t_design,t_i,t_j){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<129>";
	var t_total=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<130>";
	if(t_i>0 && dbg_array(dbg_array(t_design,t_i-1)[dbg_index],t_j)[dbg_index]=="X"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<131>";
		t_total+=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<133>";
	if(t_i<t_design.length-1 && dbg_array(dbg_array(t_design,t_i+1)[dbg_index],t_j)[dbg_index]=="X"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<134>";
		t_total+=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<136>";
	if(t_j>0 && dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j-1)[dbg_index]=="X"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<137>";
		t_total+=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<139>";
	if(t_j<dbg_array(t_design,0)[dbg_index].length-1 && dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j+1)[dbg_index]=="X"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<140>";
		t_total+=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<142>";
	if(t_i>0 && t_j<dbg_array(t_design,0)[dbg_index].length-1 && dbg_array(dbg_array(t_design,t_i-1)[dbg_index],t_j+1)[dbg_index]=="X"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<143>";
		t_total+=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<145>";
	if(t_i>0 && t_j>0 && dbg_array(dbg_array(t_design,t_i-1)[dbg_index],t_j-1)[dbg_index]=="X"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<146>";
		t_total+=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<148>";
	if(t_i<t_design.length-1 && t_j<dbg_array(t_design,0)[dbg_index].length-1 && dbg_array(dbg_array(t_design,t_i+1)[dbg_index],t_j+1)[dbg_index]=="X"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<149>";
		t_total+=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<151>";
	if(t_i<t_design.length-1 && t_j>0 && dbg_array(dbg_array(t_design,t_i+1)[dbg_index],t_j-1)[dbg_index]=="X"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<152>";
		t_total+=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<155>";
	pop_err();
	return t_total;
}
c_Level.prototype.p_generateCellularly=function(t_design){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<96>";
	var t_adjacentWalls=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<97>";
	var t_result=this.p_setArray(t_design.length,dbg_array(t_design,0)[dbg_index].length);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<99>";
	for(var t_i=0;t_i<t_design.length;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<100>";
		for(var t_j=0;t_j<dbg_array(t_design,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<101>";
			t_adjacentWalls=this.p_checkWalls(t_design,t_i,t_j);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<103>";
			if(dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]=="X"){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<104>";
				if(t_adjacentWalls>3){
					err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<105>";
					dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="X";
				}else{
					err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<107>";
					dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="O";
				}
			}else{
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<109>";
				if(dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]=="O"){
					err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<110>";
					if(t_adjacentWalls>4){
						err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<111>";
						dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="X";
					}else{
						err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<113>";
						dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="O";
					}
				}
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<117>";
			if(t_i==0 || t_j==0 || t_i==t_design.length-1 || t_j==dbg_array(t_design,0)[dbg_index].length-1){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<118>";
				dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]="X";
			}
		}
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<123>";
	pop_err();
	return t_result;
}
c_Level.prototype.p_fillCells=function(t_design){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<176>";
	for(var t_i=0;t_i<t_design.length;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<177>";
		for(var t_j=0;t_j<dbg_array(t_design,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<178>";
			dbg_array(dbg_array(t_design,t_i)[dbg_index],t_j)[dbg_index]="X";
		}
	}
	pop_err();
	return 0;
}
c_Level.prototype.p_drunkWalk=function(t_design){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<185>";
	var t_target=(t_design.length*dbg_array(t_design,0)[dbg_index].length)*0.35;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<186>";
	var t_cleared=1;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<188>";
	var t_tempX=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<189>";
	var t_tempY=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<190>";
	var t_direction=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<192>";
	var t_followBias=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<193>";
	t_tempX=((bb_random_Rnd2(1.0,(t_design.length-1)))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<194>";
	t_tempY=((bb_random_Rnd2(1.0,(dbg_array(t_design,0)[dbg_index].length-1)))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<196>";
	dbg_array(dbg_array(t_design,t_tempX)[dbg_index],t_tempY)[dbg_index]="O";
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<198>";
	while((t_cleared)<t_target){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<199>";
		if(t_followBias<40){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<200>";
			t_direction=((bb_random_Rnd2(0.0,4.0))|0);
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<202>";
		if(t_direction==0 && t_tempY>1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<203>";
			t_tempY-=1;
		}else{
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<204>";
			if(t_direction==1 && t_tempY<dbg_array(t_design,0)[dbg_index].length-2){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<205>";
				t_tempY+=1;
			}else{
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<206>";
				if(t_direction==2 && t_tempX>1){
					err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<207>";
					t_tempX-=1;
				}else{
					err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<208>";
					if(t_direction==3 && t_tempX<t_design.length-2){
						err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<209>";
						t_tempX+=1;
					}
				}
			}
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<212>";
		if(dbg_array(dbg_array(t_design,t_tempX)[dbg_index],t_tempY)[dbg_index]=="X"){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<213>";
			dbg_array(dbg_array(t_design,t_tempX)[dbg_index],t_tempY)[dbg_index]="O";
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<214>";
			t_cleared+=1;
		}
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<216>";
		t_followBias=((bb_random_Rnd2(0.0,100.0))|0);
	}
	pop_err();
	return 0;
}
c_Level.m_new=function(t_x,t_y,t_w,t_h,t_type){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<21>";
	dbg_object(this).m_generated=false;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<22>";
	dbg_object(this).m_xCoord=t_x;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<23>";
	dbg_object(this).m_yCoord=t_y;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<25>";
	dbg_object(this).m_width=t_w;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<26>";
	dbg_object(this).m_height=t_h;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<27>";
	dbg_object(this).m_layout=this.p_setArray(dbg_object(this).m_width,dbg_object(this).m_height);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<29>";
	if(t_type=="Cellular"){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<30>";
		this.p_randomlyAssignCells(dbg_object(this).m_layout);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<31>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<32>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<33>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<34>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<35>";
		this.m_layout=this.p_generateCellularly(dbg_object(this).m_layout);
	}else{
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<36>";
		if(t_type=="Drunk"){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<37>";
			this.p_fillCells(dbg_object(this).m_layout);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<38>";
			this.p_drunkWalk(dbg_object(this).m_layout);
		}
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<41>";
	dbg_object(this).m_generated=true;
	pop_err();
	return this;
}
c_Level.m_new2=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/level.monkey<6>";
	pop_err();
	return this;
}
function bb_random_Rnd(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/random.monkey<21>";
	bb_random_Seed=bb_random_Seed*1664525+1013904223|0;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/random.monkey<22>";
	var t_=(bb_random_Seed>>8&16777215)/16777216.0;
	pop_err();
	return t_;
}
function bb_random_Rnd2(t_low,t_high){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/random.monkey<30>";
	var t_=bb_random_Rnd3(t_high-t_low)+t_low;
	pop_err();
	return t_;
}
function bb_random_Rnd3(t_range){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/monkey/random.monkey<26>";
	var t_=bb_random_Rnd()*t_range;
	pop_err();
	return t_;
}
function bb_input_KeyDown(t_key){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/mojo/input.monkey<40>";
	var t_=((bb_input_device.p_KeyDown(t_key))?1:0);
	pop_err();
	return t_;
}
var bb_noisetestscene_textures=null;
var bb_noisetestscene_enemies=null;
function bb_noisetestscene_setArray(t_i,t_j){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<481>";
	var t_result=new_array_array(t_i);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<483>";
	for(var t_index=0;t_index<t_i;t_index=t_index+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<484>";
		dbg_array(t_result,t_index)[dbg_index]=new_number_array(t_j);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<487>";
	pop_err();
	return t_result;
}
function c_SimplexNoise(){
	Object.call(this);
	this.m_grad3=[];
	this.m_p=[];
	this.m_perm=[];
}
c_SimplexNoise.m_new=function(){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<18>";
	this.m_grad3=[[1,1,0],[-1,1,0],[1,-1,0],[-1,-1,0],[1,0,1],[-1,0,1],[1,0,-1],[-1,0,-1],[0,1,1],[0,-1,1],[0,1,-1],[0,-1,-1]];
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<31>";
	this.m_p=[151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180];
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<32>";
	this.m_perm=new_number_array(512);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<33>";
	for(var t_i=0;t_i<512;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<34>";
		dbg_array(this.m_perm,t_i)[dbg_index]=dbg_array(this.m_p,t_i&255)[dbg_index];
	}
	pop_err();
	return this;
}
c_SimplexNoise.prototype.p_setArray=function(t_i,t_j){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<235>";
	var t_result=new_array_array(t_i);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<237>";
	for(var t_index=0;t_index<t_i;t_index=t_index+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<238>";
		dbg_array(t_result,t_index)[dbg_index]=new_number_array(t_j);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<241>";
	pop_err();
	return t_result;
}
c_SimplexNoise.prototype.p_dot=function(t_g,t_x,t_y){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<43>";
	var t_=(dbg_array(t_g,0)[dbg_index])*t_x+(dbg_array(t_g,1)[dbg_index])*t_y;
	pop_err();
	return t_;
}
c_SimplexNoise.prototype.p_makeNoise=function(t_x,t_y){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<52>";
	var t_n0=.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<53>";
	var t_n1=.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<54>";
	var t_n2=.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<56>";
	var t_f2=0.5*(Math.sqrt(3.0)-1.0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<57>";
	var t_s=(t_x+t_y)*t_f2;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<58>";
	var t_i=((Math.floor(t_x+t_s))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<59>";
	var t_j=((Math.floor(t_y+t_s))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<61>";
	var t_g2=(3.0-Math.sqrt(3.0))/6.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<62>";
	var t_t=(t_i+t_j)*t_g2;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<63>";
	var t_x0=(t_i)-t_t;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<64>";
	var t_y0=(t_j)-t_t;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<65>";
	var t_x01=t_x-t_x0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<66>";
	var t_y01=t_y-t_y0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<69>";
	var t_i1=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<70>";
	var t_j1=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<71>";
	if(t_x01>t_y01){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<72>";
		t_i1=1;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<73>";
		t_j1=0;
	}else{
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<75>";
		t_i1=0;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<76>";
		t_j1=1;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<79>";
	var t_x1=t_x01-(t_i1)+t_g2;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<80>";
	var t_y1=t_y01-(t_j1)+t_g2;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<81>";
	var t_x2=t_x01-1.0+2.0*t_g2;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<82>";
	var t_y2=t_y01-1.0+2.0*t_g2;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<85>";
	var t_ii=t_i&255;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<86>";
	var t_jj=t_j&255;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<88>";
	var t_gi0=dbg_array(this.m_perm,t_ii+dbg_array(this.m_perm,t_jj)[dbg_index])[dbg_index] % 12;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<89>";
	var t_gi1=dbg_array(this.m_perm,t_ii+t_i1+dbg_array(this.m_perm,t_jj+t_j1)[dbg_index])[dbg_index] % 12;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<90>";
	var t_gi2=dbg_array(this.m_perm,t_ii+1+dbg_array(this.m_perm,t_jj+1)[dbg_index])[dbg_index] % 12;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<93>";
	var t_t0=0.5-t_x01*t_x01-t_y01*t_y01;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<94>";
	if(t_t0<0.0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<95>";
		t_n0=0.0;
	}else{
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<97>";
		t_t0*=t_t0;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<98>";
		t_n0=t_t0*t_t0*this.p_dot(dbg_array(this.m_grad3,t_gi0)[dbg_index],t_x01,t_y01);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<101>";
	var t_t1=0.5-t_x1*t_x1-t_y1*t_y1;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<102>";
	if(t_t1<0.0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<103>";
		t_n1=0.0;
	}else{
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<105>";
		t_t1*=t_t1;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<106>";
		t_n1=t_t1*t_t1*this.p_dot(dbg_array(this.m_grad3,t_gi1)[dbg_index],t_x1,t_y1);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<109>";
	var t_t2=0.5-t_x2*t_x2-t_y2*t_y2;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<110>";
	if(t_t2<0.0){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<111>";
		t_n2=0.0;
	}else{
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<113>";
		t_t2*=t_t2;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<114>";
		t_n2=t_t2*t_t2*this.p_dot(dbg_array(this.m_grad3,t_gi2)[dbg_index],t_x2,t_y2);
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<119>";
	var t_=70.0*(t_n0+t_n1+t_n2);
	pop_err();
	return t_;
}
c_SimplexNoise.prototype.p_makeOctavedNoise=function(t_octaves,t_roughness,t_scale,t_x,t_y,t_offset){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<215>";
	var t_noiseSum=0.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<216>";
	var t_layerFrequency=t_scale;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<217>";
	var t_layerWeight=1.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<218>";
	var t_weightSum=0.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<222>";
	for(var t_i=0;t_i<t_octaves;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<223>";
		t_noiseSum+=this.p_makeNoise((t_x+t_offset)*t_layerFrequency,(t_y+t_offset)*t_layerFrequency)*t_layerWeight;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<224>";
		t_layerFrequency=t_layerFrequency*2.0;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<225>";
		t_weightSum+=t_layerWeight;
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<226>";
		t_layerWeight*=t_roughness;
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<228>";
	var t_=t_noiseSum/t_weightSum;
	pop_err();
	return t_;
}
c_SimplexNoise.prototype.p_makeIsland=function(t_noiseMap,t_width,t_height){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<186>";
	var t_island=this.p_setArray(t_width,t_height);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<188>";
	var t_centerX=((t_width/2)|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<189>";
	var t_centerY=((t_height/2)|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<192>";
	print("Island CenterX = "+String(t_centerX));
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<193>";
	print("Island CenterY = "+String(t_centerY));
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<194>";
	var t_xDist=0.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<195>";
	var t_yDist=0.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<196>";
	var t_totalDist=0.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<198>";
	for(var t_i=0;t_i<t_width;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<199>";
		for(var t_j=0;t_j<t_height;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<200>";
			t_xDist=((t_centerX-t_i)*(t_centerX-t_i));
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<201>";
			t_yDist=((t_centerY-t_j)*(t_centerY-t_j));
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<203>";
			t_totalDist=Math.sqrt(t_xDist+t_yDist)/(t_width);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<204>";
			dbg_array(dbg_array(t_noiseMap,t_i)[dbg_index],t_j)[dbg_index]-=t_totalDist;
		}
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<207>";
	pop_err();
	return t_noiseMap;
}
c_SimplexNoise.prototype.p_generateOctavedNoiseMap=function(t_width,t_height,t_octaves,t_roughness,t_scale){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<151>";
	var t_result=this.p_setArray(t_width,t_height);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<152>";
	bb_random_Seed=bb_app_Millisecs();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<153>";
	var t_offset=((bb_random_Rnd2(0.0,1000000.0))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<155>";
	var t_layerFrequency=t_scale;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<156>";
	var t_layerWeight=1.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<157>";
	var t_weightSum=0.0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<161>";
	for(var t_i=0;t_i<t_width;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<162>";
		for(var t_j=0;t_j<t_height;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<163>";
			dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]=this.p_makeOctavedNoise(5,0.5,0.01,t_i,t_j,t_offset);
		}
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<174>";
	print("Islandizing noise...");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<175>";
	t_result=this.p_makeIsland(t_result,t_width,t_height);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<176>";
	print("Returning Noise Map");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<179>";
	pop_err();
	return t_result;
}
c_SimplexNoise.prototype.p_generateNoiseMap=function(t_width,t_height){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<128>";
	var t_result=this.p_setArray(t_width,t_height);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<129>";
	bb_random_Seed=bb_app_Millisecs();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<130>";
	var t_offset=((bb_random_Rnd2(0.0,1000.0))|0);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<131>";
	var t_frequency=5.0/(t_width);
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<133>";
	for(var t_i=0;t_i<t_width;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<134>";
		for(var t_j=0;t_j<t_height;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<135>";
			dbg_array(dbg_array(t_result,t_i)[dbg_index],t_j)[dbg_index]=this.p_makeNoise((t_i+t_offset)*t_frequency,(t_j+t_offset)*t_frequency);
		}
	}
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<141>";
	print("Returning Moisture Map");
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/simplexnoise.monkey<142>";
	pop_err();
	return t_result;
}
function bb_noisetestscene_randomlyAssignCells(t_cells,t_threshold){
	push_err();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<459>";
	bb_random_Seed=bb_app_Millisecs();
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<460>";
	var t_rand=0;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<461>";
	var t_thresh=t_threshold % 100;
	err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<463>";
	for(var t_i=0;t_i<t_cells.length;t_i=t_i+1){
		err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<464>";
		for(var t_j=0;t_j<dbg_array(t_cells,0)[dbg_index].length;t_j=t_j+1){
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<465>";
			t_rand=((bb_random_Rnd2(0.0,100.0))|0);
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<466>";
			if(t_rand<t_thresh){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<467>";
				dbg_array(dbg_array(t_cells,t_i)[dbg_index],t_j)[dbg_index]=1;
			}else{
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<469>";
				dbg_array(dbg_array(t_cells,t_i)[dbg_index],t_j)[dbg_index]=-1;
			}
			err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<471>";
			if(t_i==0 || t_j==0 || t_i==t_cells.length-1 || t_j==dbg_array(t_cells,0)[dbg_index].length-1 || dbg_array(dbg_array(t_cells,t_i)[dbg_index],t_j)[dbg_index]==9){
				err_info="/Users/BlenderChild/ProceduralContentGeneration/PCGGame/noisetestscene.monkey<472>";
				dbg_array(dbg_array(t_cells,t_i)[dbg_index],t_j)[dbg_index]=-1;
			}
		}
	}
	pop_err();
	return 0;
}
function c_iConfig(){
	Object.call(this);
}
function c_iVector2d(){
	Object.call(this);
	this.m__x=0.0;
	this.m__y=0.0;
}
c_iVector2d.m_new=function(t_x,t_y){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/vector2d.monkey<31>";
	this.m__x=t_x;
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/vector2d.monkey<32>";
	this.m__y=t_y;
	pop_err();
	return this;
}
c_iVector2d.m_new2=function(){
	push_err();
	err_info="/Applications/MonkeyXPro85e+IgnitionX2.09 orig/modules/playniax/ignitionx/math/vector2d.monkey<15>";
	pop_err();
	return this;
}
function c_iControlSet(){
	Object.call(this);
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
	bb_app2_iSpeed=1;
	bb_app2_iNextScene=null;
	bb_random_Seed=1234;
	bb_main_menu=null;
	bb_main_gameplay=null;
	bb_main_noiseTest=null;
	bb_app__updateRate=0;
	bb_globals_iEnginePointer=null;
	c_iStack2D.m__length=0;
	c_iStack2D.m__data=[];
	bb_globals_iRotation=-1;
	c_iSystemGui.m__playfield=null;
	bb_globals_iTaskList=c_iList5.m_new.call(new c_iList5);
	c_iGuiObject.m__topObject=null;
	bb_globals_iMultiTouch=8;
	c_iEngineObject.m_destroyList=c_iList6.m_new.call(new c_iList6);
	bb_contentmanager_iContent=c_iContentManager.m_new.call(new c_iContentManager);
	bb_noisetestscene_textures=null;
	bb_noisetestscene_enemies=null;
}
//${TRANSCODE_END}
