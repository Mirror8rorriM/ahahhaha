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
 let moved;for(let j=0;j<30;j++)moved=step([37,38]);assert(moved[0]>.3&&moved[1]>.4,'keyboard yaw and elevation');
 for(let j=0;j<180;j++)moved=step([38]);assert(Math.abs(moved[1]-65*Math.PI/180)<.001,'upper elevation clamp');
 step([82]);let fired=step([32],[32]);assert(fired[2]===1&&fired[8]>0,'fire projectile');
 let flying=step();assert(flying[4]>fired[4]&&flying[9]<fired[9],'ballistic integration');
 let impact=null;for(let j=0;j<120;j++){let s=step();if(s[15]>0){impact=s;break;}}
 assert(impact!==null,'impact explosion');assert(impact[12]>1.5&&impact[12]<4.2&&impact[13]>.3,'hit enemy tank, not ground');
 for(let j=0;j<12;j++)step([32]);
 let held=step([32]);assert(held[2]===0,'holding Space does not auto-fire');
 render();window.renderScene=render;window.stepScene=step;
 return {renderer:gl.getParameter(gl.RENDERER),checks:['both GLSL passes compile and link','keyboard yaw and elevation','elevation clamp','Space fires','ballistic motion with gravity','enemy collision and explosion','holding Space does not auto-fire'],impact:impact.slice(12,16),frames:frame};
}''',[ (root/'BufferA.glsl').read_text(),(root/'Image.glsl').read_text()])
 print(json.dumps(result,ensure_ascii=False,indent=2))
 page.locator('canvas').screenshot(path=str(root/'preview.png'))
 browser.close()
