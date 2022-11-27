/*-----------------------------------------------COMMON SA CODE -----------------------------------------*/

var sessionstart = false;
var hitBoolean = false;

var validBitArray = new Array();
var validTagArray = new Array();
var validDataArray = new Array ();	
var validDirtyBitArray = new Array();


var drawingSpaceHeight = 0;
var cache = 0;
var memory = 0;
var offset = 0;
var offsetrange=0;
var tag = 0;
var block = 0;
var arrowcache,v2;
var hit=0;
var listOfInstructions = new Array();
var listOfInstructionsTF = new Array();
var listOfInstructionsLS = new Array();
var step=0;
var step_store = 0;
var whichTableContainsValidTag = 0;
var replace_old_cache= false;
var store_cache_found = false;
var instructionType=0;   //SA2
var indexHighlight = 0;  //SA4

var LRU = new Array();
var LRUIndex=0;
var cacheReplacementPolicy ="FIFO";


//Drawing Properties
var topBoundAddressEvaluated, topBoundCacheTable, boxXY, indexXY, indexMid;
var min250, min220, min200;

//Logic Properties
var validindex;

function loadCommonConfiguration()
{
	offsetBit = parseInt(document.getElementById('offsetsize').value);
    offset = Math.pow(2,offsetBit);    
	cache = parseInt(document.getElementById('cachesize').value)/ offset/setAssociative;
	memory = parseInt(document.getElementById('memorysize').value);
}
function loadTableSetAssociative(){
	
	//RENDER BOTH TABLE
	for (table=0; table<setAssociative; table++)
	{
		rendertableText[table]="<table class=drawtable id=cachetable"+table+">"+
								"<tr>"+
									"<td> Index </td>"+
									"<td> Valid </td>"+
									"<td> Tag </td> "+
									"<td> Data (Hex) </td>"+
									"<td> Dirty Bit </td>"+
								"</tr>";
		for (j=0; j<cache;j++)
		{
            var tagLocal;
            if (validTagArray[j][table]!="-")
                {
                    tagLocal =  parseInt(validTagArray[j][table],2).toString(16);   
                }
            else
                {
                    tagLocal = "-"
                }
			rendertableText[table]+="<tr id=tr"+phpNaming[table]+j+"><td id=index"+phpNaming[table]+j+"> "+ j +
					" </td><td id=valid"+phpNaming[table]+j+"> "+validBitArray[j][table]+
					" </td><td id=tag"+phpNaming[table]+j+">"+ tagLocal+"</td>"+
					"<td id=offset"+phpNaming[table]+j+">"+ validDataArray[j][table]+"</td>"+
					"<td id=dirtybit"+phpNaming[table]+j+">"+ validDirtyBitArray[j][table]+"</td>"+					
					
					"</tr>";
		}
		rendertableText[table] +="</table>";
		var targettedTableFinalName = "draw" + phpNaming[table];
		document.getElementById(targettedTableFinalName).innerHTML = rendertableText[table];
	}
	resetColouring();
}


function setfirsttable(){
	sessionstart=true; 
	setFirstTableGeneric(tagBit, offsetBit);
	loadTableSetAssociative();
	document.getElementById("indexbit").innerHTML= cacheBit + " bit";

}


function highlight(attr,color){
	for (table=0; table<setAssociative;table++)
	{
		var findthevalid = attr+phpNaming[table]+validindex;
		document.getElementById(findthevalid).style.backgroundColor =color;
	}
}
function getDrawingProperties(){
    window.scroll(0,0);
	boxXY = document.getElementById("drawingSpace").getBoundingClientRect();
	topBoundAddressEvaluated = document.getElementById("addressevaluated").getBoundingClientRect().top;
	topBoundCacheTable = document.getElementById("container").getBoundingClientRect().top - boxXY.top;
	indexXY = document.getElementById("index").getBoundingClientRect();
	indexMid = (indexXY.right + indexXY.left)/2 - boxXY.left;
}

function setConst(){
    min270 = drawingSpaceHeight - 270;
    min250 = drawingSpaceHeight - 250;
    min200 = drawingSpaceHeight-200;
    min220 = drawingSpaceHeight - 220;
    min240 = drawingSpaceHeight - 240;
    min90 = drawingSpaceHeight-90;
	min290 = drawingSpaceHeight - 290;
}

function step1(){
    window.scroll(0,0);
    document.getElementById("information_text").innerHTML ="Index requested will be searched in cache as highlighted in yellow";
    document.getElementById("tag").style.backgroundColor ="";
    document.getElementById("index").style.backgroundColor="Yellow";
    document.getElementById("offset").style.backgroundColor="";
    document.getElementById("information_text").style.backgroundColor="Yellow";

    var findtherow = "tr"+phpNaming[0]+validindex;
    v2 = document.getElementById(findtherow).getBoundingClientRect().top - boxXY.top+10;
    var path = "M "+indexMid+","+topBoundAddressEvaluated+" V "+topBoundCacheTable+" H 10 V "+ v2 + "H 40";
    arrowcache = "<svg width = 100% height=100%><path d='"+path+"' stroke='red' stroke-width='1.25' fill='none'/>";
    document.getElementById("drawingSpace").innerHTML = arrowcache+"</svg>";
    highlight("tr","yellow");
}

function LRUController(){
		
	
}
function updateHitMissLabel(){
			var hitRate = hit / listOfInstructions.length;
		document.getElementById('hitRateLabel').innerHTML=  Math.round(hitRate*100,2) +"%";
		document.getElementById('missRateLabel').innerHTML= Math.round((1 - hitRate)*100,2) + "%" ;	
}
/*--------------------------------------------END OF COMMON SA CODE ------------------------------------------*/