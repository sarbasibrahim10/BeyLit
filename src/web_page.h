#pragma once
#include <pgmspace.h>

// Read-only mirror of the OLED verdict, served by the ESP32's own access point (no internet, no CDN).
// Polls /status every 250 ms; after 1.5 s without an update it shows ?UNKNOWN / NO LINK, never the last verdict.
static const char WEB_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>BeyLit Display</title>
<style>
:root{color-scheme:dark}
body{margin:0;background:#111;color:#eee;font-family:ui-monospace,Menlo,Consolas,monospace;
display:flex;flex-direction:column;align-items:center;padding:16px}
#oled{width:min(100%,560px);aspect-ratio:2/1;background:#000;color:#fff;border:3px solid #333;border-radius:8px;
padding:4% 5%;box-sizing:border-box;display:flex;flex-direction:column;justify-content:space-between}
#oled.inv{background:#fff;color:#000}
#oled.blink{animation:b .5s steps(1) infinite}
@keyframes b{50%{background:#000;color:#fff}}
#word{font-size:clamp(30px,10vw,68px);font-weight:700;letter-spacing:.04em;line-height:1}
.l{font-size:clamp(13px,3.8vw,24px)}
.row{display:flex;justify-content:space-between;gap:8px}
#foot{opacity:.75}
#link{margin-top:12px;font-size:14px;color:#aaa}
.note{max-width:560px;font-size:13px;color:#999;margin-top:8px;text-align:center;line-height:1.4}
</style></head><body>
<div id="oled">
<div id="word">?UNKNOWN</div>
<div class="l" id="tilt">Pitch -- Roll --</div>
<div class="l row"><span id="dist">Dist --</span><span id="lvl"></span></div>
<div class="l" id="reason">CONNECTING</div>
<div class="l" id="foot">ADVISORY-DEMO LIMITS</div>
</div>
<div id="link">connecting...</div>
<p class="note">Mirror of the BeyLit OLED. Advisory prototype only: no motor, brake or steering control.
Limits are unvalidated demo values. If updates stop, this page shows ?UNKNOWN, never the last verdict.</p>
<script>
const $=id=>document.getElementById(id);let last=0;
const f=v=>(v>=0?'+':'')+v.toFixed(1);
function render(s){
 $('word').textContent=s.word;
 $('tilt').textContent=s.pitch==null?'Pitch -- Roll --':'Pitch '+f(s.pitch)+' Roll '+f(s.roll);
 $('dist').textContent=s.dist_cm==null?'Dist --':'Dist '+s.dist_cm+' cm';
 $('lvl').textContent=s.leveling;$('reason').textContent=s.reason;
 const o=$('oled');o.classList.toggle('inv',s.level>=4);o.classList.toggle('blink',s.level==5);
 $('link').textContent='live - ESP32 uptime '+Math.round(s.uptime_ms/1000)+' s';
}
function lost(){
 $('word').textContent='?UNKNOWN';$('reason').textContent='NO LINK TO ESP32';
 $('tilt').textContent='Pitch -- Roll --';$('dist').textContent='Dist --';$('lvl').textContent='';
 $('oled').classList.remove('inv','blink');
 $('link').textContent=last?'no update for '+((Date.now()-last)/1000|0)+' s':'connecting...';
}
async function poll(){
 const c=new AbortController();const t=setTimeout(()=>c.abort(),900);
 try{const r=await fetch('/status',{cache:'no-store',signal:c.signal});render(await r.json());last=Date.now();}catch(e){}
 clearTimeout(t);if(Date.now()-last>1500)lost();setTimeout(poll,250);
}
poll();
</script></body></html>)HTML";
