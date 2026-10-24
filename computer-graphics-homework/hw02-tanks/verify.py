from pathlib import Path
from playwright.sync_api import sync_playwright
import json
import shutil
root=Path(__file__).resolve().parent
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path=shutil.which('chromium'),headless=True,args=['--no-sandbox','--use-gl=angle','--use-angle=swiftshader','--enable-unsafe-swiftshader'])
 page=browser.new_page(viewport={'width':960,'height':540})
 page.set_content('<style>body{margin:0}canvas{width:960px;height:540px}</style><canvas width="640" height="360"></canvas>')
 result=page.evaluate('''async ([commonSource,bufferSource,imageSource]) => {
 const canvas=document.querySelector('canvas'),gl=canvas.getContext('webgl2',{preserveDrawingBuffer:true});
 if(!gl) throw Error('WebGL2 unavailable');
 if(!gl.getExtension('EXT_color_buffer_float')) throw Error('Float render targets unavailable');
 const w=640,h=360;
 const prefix='#version 300 es\\nprecision highp float;\\nuniform vec3 iResolution; uniform float iTime, iTimeDelta; uniform int iFrame; uniform vec4 iMouse; uniform sampler2D iChannel0,iChannel1; out vec4 outputColor;\\n';
 function shader(kind,source){let s=gl.createShader(kind);gl.shaderSource(s,source);gl.compileShader(s);if(!gl.getShaderParameter(s,gl.COMPILE_STATUS))throw Error(gl.getShaderInfoLog(s));return s;}
 const vertex=shader(gl.VERTEX_SHADER,'#version 300 es\\nvoid main(){vec2 p=vec2(float((gl_VertexID<<1)&2),float(gl_VertexID&2));gl_Position=vec4(p*2.0-1.0,0,1);}');
 function program(source){let p=gl.createProgram();gl.attachShader(p,vertex);gl.attachShader(p,shader(gl.FRAGMENT_SHADER,prefix+commonSource+'\\n'+source+'\\nvoid main(){mainImage(outputColor,gl_FragCoord.xy);}'));gl.linkProgram(p);if(!gl.getProgramParameter(p,gl.LINK_STATUS))throw Error(gl.getProgramInfoLog(p));return p;}
 const bp=program(bufferSource),ip=program(imageSource);
 function texture(width,height,floating){let t=gl.createTexture();gl.bindTexture(gl.TEXTURE_2D,t);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE);gl.texImage2D(gl.TEXTURE_2D,0,floating?gl.RGBA32F:gl.RGBA8,width,height,0,gl.RGBA,floating?gl.FLOAT:gl.UNSIGNED_BYTE,null);return t;}
 let textures=[texture(w,h,true),texture(w,h,true)],keyboard=texture(256,3,false),fbo=gl.createFramebuffer(),frame=0,current=0;
 function bind(p,name,value){gl.activeTexture(gl.TEXTURE0+value);gl.uniform1i(gl.getUniformLocation(p,name),value);}
 function uniforms(p){gl.useProgram(p);gl.uniform3f(gl.getUniformLocation(p,'iResolution'),w,h,1);gl.uniform1f(gl.getUniformLocation(p,'iTime'),frame/60);gl.uniform1f(gl.getUniformLocation(p,'iTimeDelta'),1/60);gl.uniform1i(gl.getUniformLocation(p,'iFrame'),frame);bind(p,'iChannel0',0);gl.bindTexture(gl.TEXTURE_2D,textures[current]);bind(p,'iChannel1',1);gl.bindTexture(gl.TEXTURE_2D,keyboard);}
 function step(down=[],press=[]){let bytes=new Uint8Array(256*3*4);for(let k of down)bytes[k*4]=255;for(let k of press)bytes[(256+k)*4]=255;gl.activeTexture(gl.TEXTURE1);gl.bindTexture(gl.TEXTURE_2D,keyboard);gl.texSubImage2D(gl.TEXTURE_2D,0,0,0,256,3,gl.RGBA,gl.UNSIGNED_BYTE,bytes);gl.bindFramebuffer(gl.FRAMEBUFFER,fbo);gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_2D,textures[1-current],0);if(gl.checkFramebufferStatus(gl.FRAMEBUFFER)!==gl.FRAMEBUFFER_COMPLETE)throw Error('Incomplete framebuffer');uniforms(bp);gl.viewport(0,0,w,h);gl.drawArrays(gl.TRIANGLES,0,3);current=1-current;frame++;let data=new Float32Array(272);gl.readPixels(0,0,68,1,gl.RGBA,gl.FLOAT,data);if(gl.getError()!==gl.NO_ERROR)throw Error('WebGL error');return Array.from(data);}
 function render(){gl.bindFramebuffer(gl.FRAMEBUFFER,null);uniforms(ip);gl.viewport(0,0,w,h);gl.drawArrays(gl.TRIANGLES,0,3);gl.finish();if(gl.getError()!==gl.NO_ERROR)throw Error('Image GL error');}
 const assert=(condition,label)=>{if(!condition)throw Error(label);};
 function reset(){frame=0;return step();}
 function untilImpact(){for(let j=0;j<300;j++){let s=step([32]);if(s[2]<.5)return s;}throw Error('impact timeout');}
 function imagePixels(){render();let a=new Uint8Array(w*h*4);gl.readPixels(0,0,w,h,gl.RGBA,gl.UNSIGNED_BYTE,a);return a;}
 function difference(a,b){let n=0;for(let i=0;i<a.length;i+=4)if(Math.abs(a[i]-b[i])+Math.abs(a[i+1]-b[i+1])+Math.abs(a[i+2]-b[i+2])>10)n++;return n;}
 let initial=reset();assert(Math.abs(initial[1]-Math.PI/15)<.001&&initial[12]===0,'initial state');
 let moved;for(let j=0;j<30;j++)moved=step([37,38]);assert(moved[0]>.3&&moved[1]>.4,'left/up');
 for(let j=0;j<30;j++)moved=step([39]);assert(Math.abs(moved[0])<.001,'right');
 for(let j=0;j<180;j++)moved=step([38]);assert(Math.abs(moved[1]-65*Math.PI/180)<.001,'upper clamp');
 for(let j=0;j<180;j++)moved=step([40]);assert(Math.abs(moved[1]-2*Math.PI/180)<.001,'lower clamp');
 reset();let intact=imagePixels();let fired=step([32],[32]);assert(fired[2]===1&&fired[8]>0,'fire');
 let flying=step([32]);assert(flying[4]>fired[4]&&flying[9]<fired[9],'gravity');
 let impact=untilImpact();assert(impact[12]===1&&impact[19]>=5&&impact[19]<=7,'tank crater created');
 assert(difference(intact,imagePixels())>10,'tank crater visible');
 for(let j=0;j<12;j++)step([32]);assert(step([32])[2]===0,'no autofire');
 let firstRecord=impact.slice(16,20);
 // Two hits at different ground positions must preserve the tank scar.
 for(let j=0;j<50;j++)step([39]);let beforeGround=imagePixels();step([32],[32]);let ground=untilImpact();
 assert(ground[12]===2&&ground[23]===1,'first ground crater appended');
 assert(firstRecord.every((v,i)=>v===ground[16+i]),'tank crater preserved after ground hit');
 assert(difference(beforeGround,imagePixels())>20,'ground crater visible');
 // Repeat the trajectory: the new projectile enters the old cavity.
 step([32],[32]);let deeper=untilImpact();assert(deeper[5]<0&&deeper[12]===3,'collision inside existing crater');
 for(let j=0;j<25;j++)step([39]);step([32],[32]);let multiple=untilImpact();
 assert(multiple[12]===4&&multiple[31]===1,'another crater appended');
 let records=multiple.slice(16,32);for(let j=0;j<60;j++)multiple=step();
 assert(records.every((v,i)=>v===multiple[16+i]),'all craters persist');
 // Save the genuinely simulated scene before isolated geometry probes.
 const saved=multiple.slice();
 function seed(data){gl.activeTexture(gl.TEXTURE0);gl.bindTexture(gl.TEXTURE_2D,textures[current]);gl.texSubImage2D(gl.TEXTURE_2D,0,0,0,68,1,gl.RGBA,gl.FLOAT,new Float32Array(data));}
 const probeProgram=program('uniform vec3 probePoint; void mainImage(out vec4 c,in vec2 f){loadState();c=vec4(worldScene(probePoint),1.0);}');
 function probe(point){let p=probeProgram;gl.bindFramebuffer(gl.FRAMEBUFFER,fbo);gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_2D,textures[1-current],0);uniforms(p);gl.uniform3f(gl.getUniformLocation(p,'probePoint'),...point);gl.viewport(0,0,1,1);gl.drawArrays(gl.TRIANGLES,0,3);let a=new Float32Array(4);gl.readPixels(0,0,1,1,gl.RGBA,gl.FLOAT,a);return a[0];}
 // A local scar follows the player's rotating turret.
 let data=Array(272).fill(0);data[1]=Math.PI/15;data[12]=1;data.splice(16,4,.70,.2,.3,3);seed(data);
 let d0=probe([-2.35,1.47,.3]);data[0]=Math.PI/2;seed(data);let d90=probe([-2.7,1.47,-.65]);
 assert(d0>.1&&Math.abs(d0-d90)<.002,'crater follows turret yaw');
 // The same local construction applies to hulls and barrels on both tanks.
 for(let [part,center,point] of [[2,[1.17,.63,0],[-1.88,.63,0]],[5,[1.17,.63,0],[4.12,.63,0]],[4,[1.5,.14,0],[-1.5,1.46,0]],[7,[1.5,.14,0],[1.527,1.668,0]]]){
  data=Array(272).fill(0);data[12]=1;data.splice(16,4,...center,part);seed(data);assert(probe(point)>0,'carved part '+part);
 }
 data=Array(272).fill(0);data[1]=Math.PI/6;data[12]=1;data.splice(16,4,1.5,.14,0,4);seed(data);
 assert(probe([-1.74596,2.19794,0])>0,'crater follows barrel pitch');
 // At capacity, keep all old records; do not wrap over them.
 data=Array(272).fill(0);data[1]=Math.PI/15;data[2]=1;data.splice(4,4,10,.105,10,0);data[9]=-1;data[12]=64;
 for(let i=0;i<64;i++)data.splice(16+i*4,4,1000+i,.4,1000,1);seed(data);let full=step();
 assert(full[2]===0&&full[12]===64&&full.slice(16).every((v,i)=>Math.abs(v-data[16+i])<.00001),'capacity preserves old craters');
 assert(reset()[12]===0,'reset clears all');
 seed(saved);render();
 return {renderer:gl.getParameter(gl.RENDERER),checks:['GLSL compilation','keyboard and angle limits','ballistics','visible tank crater','multiple visible ground craters','all records retained','collision with carved ground','turret and barrel attachment','both tanks hull/barrel damage','64-record capacity','reset clears all'],cratersInPreview:saved[12]};
}''',[ (root/'Common.glsl').read_text(), (root/'BufferA.glsl').read_text(),(root/'Image.glsl').read_text()])
 print(json.dumps(result,ensure_ascii=False,indent=2))
 page.locator('canvas').screenshot(path=str(root/'preview.png'))
 browser.close()
