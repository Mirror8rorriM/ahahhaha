from pathlib import Path
from playwright.sync_api import sync_playwright
import json
import shutil
root=Path(__file__).resolve().parent
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path=shutil.which('chromium'),headless=True,args=['--no-sandbox','--use-gl=angle','--use-angle=swiftshader','--enable-unsafe-swiftshader'])
 page=browser.new_page(viewport={'width':960,'height':540})
 page.set_content('<style>body{margin:0}canvas{width:960px;height:540px}</style><canvas width="640" height="360"></canvas>')
 result=page.evaluate('''async ([bufferSource,imageSource]) => {
 const canvas=document.querySelector('canvas'),gl=canvas.getContext('webgl2',{preserveDrawingBuffer:true});
 if(!gl) throw Error('WebGL2 unavailable');
 if(!gl.getExtension('EXT_color_buffer_float')) throw Error('Float render targets unavailable');
 const w=640,h=360;
 const prefix='#version 300 es\\nprecision highp float;\\nuniform vec3 iResolution; uniform float iTime, iTimeDelta; uniform int iFrame; uniform vec4 iMouse; uniform sampler2D iChannel0,iChannel1; out vec4 outputColor;\\n';
 function shader(kind,source){let s=gl.createShader(kind);gl.shaderSource(s,source);gl.compileShader(s);if(!gl.getShaderParameter(s,gl.COMPILE_STATUS))throw Error(gl.getShaderInfoLog(s));return s;}
 const vertex=shader(gl.VERTEX_SHADER,'#version 300 es\\nvoid main(){vec2 p=vec2(float((gl_VertexID<<1)&2),float(gl_VertexID&2));gl_Position=vec4(p*2.0-1.0,0,1);}');
 function program(source){let p=gl.createProgram();gl.attachShader(p,vertex);gl.attachShader(p,shader(gl.FRAGMENT_SHADER,prefix+source+'\\nvoid main(){mainImage(outputColor,gl_FragCoord.xy);}'));gl.linkProgram(p);if(!gl.getProgramParameter(p,gl.LINK_STATUS))throw Error(gl.getProgramInfoLog(p));return p;}
 const bp=program(bufferSource),ip=program(imageSource);
 function texture(width,height,floating){let t=gl.createTexture();gl.bindTexture(gl.TEXTURE_2D,t);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE);gl.texImage2D(gl.TEXTURE_2D,0,floating?gl.RGBA32F:gl.RGBA8,width,height,0,gl.RGBA,floating?gl.FLOAT:gl.UNSIGNED_BYTE,null);return t;}
 let textures=[texture(w,h,true),texture(w,h,true)],keyboard=texture(256,3,false),fbo=gl.createFramebuffer(),frame=0,current=0;
 function bind(p,name,value){gl.activeTexture(gl.TEXTURE0+value);gl.uniform1i(gl.getUniformLocation(p,name),value);}
 function uniforms(p){gl.useProgram(p);gl.uniform3f(gl.getUniformLocation(p,'iResolution'),w,h,1);gl.uniform1f(gl.getUniformLocation(p,'iTime'),frame/60);gl.uniform1f(gl.getUniformLocation(p,'iTimeDelta'),1/60);gl.uniform1i(gl.getUniformLocation(p,'iFrame'),frame);bind(p,'iChannel0',0);gl.bindTexture(gl.TEXTURE_2D,textures[current]);bind(p,'iChannel1',1);gl.bindTexture(gl.TEXTURE_2D,keyboard);}
 function step(down=[],press=[]){let bytes=new Uint8Array(256*3*4);for(let k of down)bytes[k*4]=255;for(let k of press)bytes[(256+k)*4]=255;gl.activeTexture(gl.TEXTURE1);gl.bindTexture(gl.TEXTURE_2D,keyboard);gl.texSubImage2D(gl.TEXTURE_2D,0,0,0,256,3,gl.RGBA,gl.UNSIGNED_BYTE,bytes);gl.bindFramebuffer(gl.FRAMEBUFFER,fbo);gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_2D,textures[1-current],0);if(gl.checkFramebufferStatus(gl.FRAMEBUFFER)!==gl.FRAMEBUFFER_COMPLETE)throw Error('Incomplete framebuffer');uniforms(bp);gl.viewport(0,0,w,h);gl.drawArrays(gl.TRIANGLES,0,3);current=1-current;frame++;let data=new Float32Array(16);gl.readPixels(0,0,4,1,gl.RGBA,gl.FLOAT,data);if(gl.getError()!==gl.NO_ERROR)throw Error('WebGL error');return Array.from(data);}
 function render(){gl.bindFramebuffer(gl.FRAMEBUFFER,null);uniforms(ip);gl.viewport(0,0,w,h);gl.drawArrays(gl.TRIANGLES,0,3);gl.finish();if(gl.getError()!==gl.NO_ERROR)throw Error('Image GL error');}
 const assert=(condition,label)=>{if(!condition)throw Error(label);};
 let initial=step();assert(Math.abs(initial[1]-Math.PI/15)<.001,'initial elevation');
 let moved;for(let j=0;j<30;j++)moved=step([37,38]);assert(moved[0]>.3&&moved[1]>.4,'left and up');
 for(let j=0;j<30;j++)moved=step([39]);assert(Math.abs(moved[0])<.001,'right');
 for(let j=0;j<180;j++)moved=step([38]);assert(Math.abs(moved[1]-65*Math.PI/180)<.001,'upper elevation clamp');
 for(let j=0;j<180;j++)moved=step([40]);assert(Math.abs(moved[1]-2*Math.PI/180)<.001,'lower elevation clamp');
 function reset(){frame=0;step();}
 reset();let fired=step([32],[32]);assert(fired[2]===1&&fired[8]>0,'Space fires');
 let flying=step([32]);assert(flying[4]>fired[4]&&flying[9]<fired[9],'ballistic motion');
 // Save a frame with the projectile in flight.
 render();
 const pixels=new Uint8Array(w*h*4);gl.readPixels(0,0,w,h,gl.RGBA,gl.UNSIGNED_BYTE,pixels);
 let green=0,red=0;for(let i=0;i<pixels.length;i+=4){if(pixels[i+1]>pixels[i]*1.3)green++;if(pixels[i]>pixels[i+1]*1.5)red++;}
 assert(green>100&&red>100,'both tank colors visible');
 let impact=null;for(let j=0;j<120;j++){let s=step([32]);if(s[2]<.5){impact=s;break;}}
 assert(impact!==null,'projectile deactivates on impact');assert(impact[15]===0,'tank hit makes no crater');assert(impact[4]>1.5&&impact[4]<4.2&&impact[5]>.3,'hit target before ground');
 for(let j=0;j<12;j++)step([32]);
 assert(step([32])[2]===0,'holding Space does not auto-fire');
 assert(step([32],[32])[2]===1,'new press fires again');
 reset();for(let j=0;j<50;j++)step([39]);
 render();const flat=new Uint8Array(w*h*4);gl.readPixels(0,0,w,h,gl.RGBA,gl.UNSIGNED_BYTE,flat);
 step([32],[32]);
 let ground=null;for(let j=0;j<240;j++){let s=step();if(s[2]<.5){ground=s;break;}}
 assert(ground!==null&&ground[5]<=.1,'ground collision');
 assert(ground[15]===1&&Math.abs(ground[12]-ground[4])<.001&&Math.abs(ground[14]-ground[6])<.001,'crater at impact');
 let persistent;for(let j=0;j<60;j++)persistent=step();
 assert(persistent.slice(12).every((v,i)=>v===ground[12+i]),'crater persists across frames');
 render();const carved=new Uint8Array(w*h*4);gl.readPixels(0,0,w,h,gl.RGBA,gl.UNSIGNED_BYTE,carved);
 let changed=0;for(let i=0;i<flat.length;i+=4)if(Math.abs(flat[i]-carved[i])+Math.abs(flat[i+1]-carved[i+1])+Math.abs(flat[i+2]-carved[i+2])>10)changed++;
 assert(changed>20,'crater geometry visible');
 step([32],[32]);let second=null;for(let j=0;j<240;j++){let s=step();if(s[2]<.5){second=s;break;}}
 assert(second!==null&&second[5]<0,'next projectile enters crater below original plane');
 reset();assert(step()[15]===0,'reset clears crater');
 // Leave a visible crater for the screenshot.
 for(let j=0;j<50;j++)step([39]);step([32],[32]);for(let j=0;j<240;j++)step();render();
 return {renderer:gl.getParameter(gl.RENDERER),checks:['both GLSL passes compile and link','left/right/up/down','both elevation limits','Space fires and refires','ballistic motion with gravity','target collision','ground collision','persistent crater at impact','crater geometry rendered','next shot collides with crater','reset clears crater','tank hit makes no crater','holding Space does not auto-fire','both tanks rendered'],impact:impact.slice(4,7),frames:frame};
}''',[ (root/'BufferA.glsl').read_text(),(root/'Image.glsl').read_text()])
 print(json.dumps(result,ensure_ascii=False,indent=2))
 page.locator('canvas').screenshot(path=str(root/'preview.png'))
 browser.close()
