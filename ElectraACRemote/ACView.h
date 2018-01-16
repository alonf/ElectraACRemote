#pragma once
#include <String>
#ifndef _ACVIEW_h
#define _ACVIEW_h

const static char ACViewHtmlTemplate[] PROGMEM = R"(
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title><%=Title%></title>
<link rel="stylesheet" type="text/css" href="view.css" media="all">

</head>
<body id="main_body" >
	<div id="form_container">
		<h1><a><%=Title%></a></h1>
		<form id="form_39821" class="appnitro"  method="post" action="sendacstate">
					<div class="form_description">
			<p></p>
		<input type="hidden" name="ApiKey" value="<%=ApiKey%>"></input>
		</div>						
			<ul >
			<li id="li_1" >
		<label class="description" for="ACMode">Air Conditioner Mode </label>
		<span>
			<input id="ACMode_1" name="ACMode" class="element radio" type="radio" value="AUTO" <%=ACModeAuto%>/>
<label class="choice" for="ACMode_1">Auto</label>
		    <input id="ACMode_2" name="ACMode" class="element radio" type="radio" value="COOL" <%=ACModeCool%> />
<label class="choice" for="ACMode_2">Cool</label>
<input id="ACMode_3" name="ACMode" class="element radio" type="radio" value="HEAT" <%=ACModeHeat%>/>
<label class="choice" for="ACMode_3">Heat</label>
<input id="ACMode_4" name="ACMode" class="element radio" type="radio" value="FAN" <%=ACModeFan%> />
<label class="choice" for="ACMode_4">Fan</label>
<input id="ACMode_5" name="ACMode" class="element radio" type="radio" value="DRY" <%=ACModeDry%>/>
<label class="choice" for="ACMode_5">Dry</label>

		</span> 
		</li>		<li id="li_2" >
		<label class="description" for="ACFan_2">Fan Speed </label>
		<span>
			<input id="ACFan_1" name="ACFan" class="element radio" type="radio" value="AUTO" <%=FanModeAuto%> />
<label class="choice" for="ACFan_1">Auto</label>
<input id="ACFan_2" name="ACFan" class="element radio" type="radio" value="LOW" <%=FanModeLow%> />
<label class="choice" for="ACFan_2">Low</label>
<input id="ACFan_3" name="ACFan" class="element radio" type="radio" value="MEDIUM" <%=FanModeMedium%> />
<label class="choice" for="ACFan_3">Medium</label>
<input id="ACFan_4" name="ACFan" class="element radio" type="radio" value="HIGH" <%=FanModeHigh%> />
<label class="choice" for="ACFan_4">High</label>

		</span> 
		</li>		<li id="li_3" >
		<label class="description" for="element_3">Operations </label>
		<span>
			<input id="IsSwingOn" name="IsSwingOn" class="element checkbox" type="checkbox" value="true" <%=Swing%>/>
<label class="choice" for="element_3_1">Swing</label>
			<input type="hidden" name="IsSwingOn" value="false" />
<input id="IsSleepOn" name="IsSleepOn" class="element checkbox" type="checkbox" value="true" <%=Sleep%> />
<label class="choice" for="element_3_2">Sleep</label>
			<input type="hidden" name="IsSleepOn" value="false" />
<input id="IsSIFeelOn" name="IsiFeelOn" class="element checkbox" type="checkbox" value="true" <%=iFeel%> />
<label class="choice" for="element_3_2">iFeel</label>
			<input type="hidden" name="IsiFeelOn" value="false" />
<input id="IsPowerToggle" name="IsPowerToggle" class="element checkbox" type="checkbox" value="true" <%=PowerToggle%> />
<label class="choice" for="element_3_2">Toggle Power</label>
			<input type="hidden" name="IsPowerToggle" value="false" />

		</span> 
		</li>		<li id="li_4" >
		<label class="description" for="element_4">Temperature </label>
		<div>
			<input id="Temperature" name="Temperature" class="element text medium" type="text" maxlength="255" value="<%=Temperature%>"/> 
		</div> 
		</li>
			
					<li class="buttons">
			    <input type="hidden" name="form_id" value="39821" />
			    
				<input id="saveForm" class="button_text" type="submit" name="submit" value="Submit" />
		</li>
			</ul>
			<input type="hidden" name="IsPowerOn" value="true" />
		</form>	
	</div>
	</body>
</html>
)";


#endif
