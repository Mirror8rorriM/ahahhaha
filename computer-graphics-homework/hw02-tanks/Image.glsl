// Image: iChannel0 = Buffer A. No other channels required.
// Quaternion convention: q.xyz is the imaginary part, q.w is the real part.
const float PI = 3.14159265359;
vec4 controls, projectile, explosion;
vec4 axisAngle(vec3 axis, float angle) {
    return vec4(normalize(axis) * sin(angle * .5), cos(angle * .5));
}
vec4 qMul(vec4 a, vec4 b) {
    return vec4(a.w*b.xyz + b.w*a.xyz + cross(a.xyz,b.xyz),
                a.w*b.w - dot(a.xyz,b.xyz));
}
vec3 qRotate(vec4 q, vec3 p) {
    // Equivalent to q * (p,0) * conjugate(q) for a unit quaternion.
    return p + 2.0 * cross(q.xyz, cross(q.xyz,p) + q.w*p);
}
vec3 toLocal(vec4 q, vec3 p) { return qRotate(vec4(-q.xyz,q.w),p); }
float sdBox(vec3 p, vec3 b) {
    vec3 d=abs(p)-b;
    return length(max(d,0.0))+min(max(d.x,max(d.y,d.z)),0.0);
}
float cylinderX(vec3 p, float halfLength, float radius) {
    vec2 d=vec2(abs(p.x)-halfLength,length(p.yz)-radius);
    return min(max(d.x,d.y),0.0)+length(max(d,0.0));
}
vec2 nearer(vec2 a, vec2 b) { return a.x < b.x ? a : b; }
vec2 tank(vec3 p, vec3 center, float yaw, float elevation, float colorId) {
    p-=center;
    vec2 result=vec2(sdBox(p-vec3(0,.63,0),vec3(1.12,.43,.72))-.05,colorId);
    // Two tracks and visible road wheels on each side.
    for(int side=-1;side<=1;side+=2) {
        float z=float(side)*.78;
        result=nearer(result,vec2(sdBox(p-vec3(0,.34,z),vec3(1.20,.28,.19))-.07,3));
        for(int j=0;j<5;++j) {
            vec3 wheel=p-vec3(-.88+float(j)*.44,.34,z+float(side)*.18);
            float d=cylinderX(wheel.zyx,.035,.205);
            result=nearer(result,vec2(d,4));
        }
    }
    vec4 yawQ=axisAngle(vec3(0,1,0),yaw);
    vec3 local=toLocal(yawQ,p-vec3(0,1.27,0));
    result=nearer(result,vec2(sdBox(local,vec3(.65,.30,.53))-.06,colorId));
    result=nearer(result,vec2(cylinderX((local-vec3(-.15,.34,0)).yxz,.05,.24),4));
    // Pitch around local Z, followed by yaw around world Y. Order matters.
    vec4 barrelQ=qMul(yawQ,axisAngle(vec3(0,0,1),elevation));
    vec3 barrel=toLocal(barrelQ,p-vec3(0,1.37,0));
    result=nearer(result,vec2(cylinderX(barrel-vec3(1.05,0,0),.79,.09),4));
    result=nearer(result,vec2(cylinderX(barrel-vec3(1.78,0,0),.13,.135),4));
    return result;
}
vec2 scene(vec3 p) {
    vec2 hit=vec2(p.y,0);
    hit=nearer(hit,tank(p,vec3(-3,0,0),controls.x,controls.y,1.0));
    hit=nearer(hit,tank(p,vec3(3,0,0),PI,radians(8.0),2.0));
    if(controls.z>.5) hit=nearer(hit,vec2(length(p-projectile.xyz)-.10,5));
    return hit;
}
vec3 normalAt(vec3 p) {
    const vec2 e=vec2(.001,0);
    return normalize(vec3(scene(p+e.xyy).x-scene(p-e.xyy).x,
                          scene(p+e.yxy).x-scene(p-e.yxy).x,
                          scene(p+e.yyx).x-scene(p-e.yyx).x));
}
float shadow(vec3 p,vec3 light) {
    float shade=1.0,t=.04;
    for(int j=0;j<36;++j) {
        float d=scene(p+light*t).x;
        shade=min(shade,12.0*d/t);
        t+=clamp(d,.04,.6);
        if(d<.001||t>18.0) break;
    }
    return clamp(shade,.18,1.0);
}
vec3 material(float id,vec3 p) {
    if(id<.5) {
        float tile=mod(floor(p.x)+floor(p.z),2.0);
        return mix(vec3(.21,.24,.16),vec3(.27,.30,.20),tile);
    }
    if(id<2.5) {
        float camo=sin(p.x*7.0+sin(p.z*8.0))*sin(p.y*11.0+p.z*4.0);
        vec3 base=id<1.5?vec3(.20,.36,.16):vec3(.48,.22,.12);
        return base*mix(.72,1.15,smoothstep(-.2,.2,camo));
    }
    if(id<3.5) return vec3(.065,.073,.065);
    if(id<4.5) return vec3(.19,.22,.18);
    return vec3(1.0,.6,.10);
}
float segmentDistance(vec2 p,vec2 a,vec2 b) {
    vec2 d=b-a;
    return length(p-a-d*clamp(dot(p-a,d)/max(dot(d,d),.00001),0.0,1.0));
}
void mainImage(out vec4 fragColor,in vec2 fragCoord) {
    controls=texelFetch(iChannel0,ivec2(0,0),0);
    projectile=texelFetch(iChannel0,ivec2(1,0),0);
    explosion=texelFetch(iChannel0,ivec2(3,0),0);
    vec2 uv=(2.0*fragCoord-iResolution.xy)/iResolution.y;
    // Orbit camera. Its local ray implements a perspective projection (FOV 48 deg).
    float azimuth=.75, altitude=.38;
    if(iMouse.z>0.0) {
        azimuth=.75+(iMouse.x/iResolution.x-.5)*2.0*PI;
        altitude=clamp(.38+(iMouse.y/iResolution.y-.5)*1.2,.12,1.25);
    }
    vec3 target=vec3(0,.65,0);
    vec3 camera=target+12.5*vec3(cos(altitude)*sin(azimuth),sin(altitude),cos(altitude)*cos(azimuth));
    vec3 forward=normalize(target-camera);
    vec3 right=normalize(cross(forward,vec3(0,1,0)));
    vec3 up=cross(right,forward);
    mat3 cameraToWorld=mat3(right,up,forward);
    vec3 ray=cameraToWorld*normalize(vec3(uv*tan(radians(48.0)*.5),1));
    vec3 sky=mix(vec3(.62,.73,.77),vec3(.24,.42,.59),clamp(ray.y*.9+.3,0.0,1.0));
    vec3 color=sky;
    float t=0.0; vec2 hit=vec2(0);
    bool found=false;
    for(int j=0;j<150;++j) {
        hit=scene(camera+ray*t);
        if(hit.x<.0015) {found=true;break;}
        t+=max(hit.x*.85,.0005);
        if(t>60.0) break;
    }
    if(found) {
        vec3 p=camera+ray*t,n=normalAt(p);
        vec3 light=normalize(vec3(-.6,1,.5));
        float diffuse=max(dot(n,light),0.0);
        float sh=shadow(p+n*.012,light);
        color=material(hit.y,p)*(.27+.85*diffuse*sh);
        color+=vec3(.18)*pow(max(dot(reflect(-light,n),-ray),0.0),32.0)*sh;
        color=mix(color,sky,1.0-exp(-t*.018));
    }
    // Animated fireball on impact. Ray/sphere intersection, depth-tested.
    if(explosion.w>0.0) {
        float age=1.25-explosion.w;
        float radius=.12+1.1*sin(clamp(age/1.25,0.0,1.0)*PI*.85);
        vec3 oc=camera-explosion.xyz;
        float b=dot(oc,ray),c=dot(oc,oc)-radius*radius;
        float disc=b*b-c;
        if(disc>0.0) {
            float front=-b-sqrt(disc);
            if(front>0.0 && (!found||front<t)) {
                vec3 p=camera+ray*front-explosion.xyz;
                float noise=sin(p.x*19.0+age*17.0)*sin(p.y*23.0-age*13.0)*sin(p.z*17.0);
                vec3 fire=mix(vec3(1,.12,.015),vec3(1,.85,.18),.5+.5*noise);
                fire=mix(fire,vec3(.18,.15,.13),smoothstep(.4,1.25,age));
                color=mix(color,fire,clamp(explosion.w*1.8,0.0,1.0));
            }
        }
    }
    color=pow(max(color,0.0),vec3(1.0/2.2));
    // Minimal HUD: azimuth compass, elevation gauge and projectile ready light.
    vec2 screen=fragCoord/iResolution.xy;
    vec2 h=(fragCoord-vec2(70,70))/iResolution.y;
    float ring=abs(length(h)-.065);
    float needle=segmentDistance(h,vec2(0),.055*vec2(cos(controls.x),sin(controls.x)));
    float hud=1.0-smoothstep(.001,.003,min(ring,needle));
    color=mix(color,vec3(.65,1,.65),hud*.9);
    float gauge=step(.93,screen.x)*step(screen.x,.945)*step(.06,screen.y)*step(screen.y,.26);
    color=mix(color,vec3(.08),gauge*.75);
    float level=.06+.20*(controls.y-radians(2.0))/radians(63.0);
    color=mix(color,vec3(.65,1,.65),gauge*step(screen.y,level));
    float ready=1.0-smoothstep(.006,.009,length((fragCoord-vec2(70,125))/iResolution.y));
    color=mix(color,controls.z>.5?vec3(1,.4,.15):vec3(.4,1,.4),ready);
    fragColor=vec4(color,1);
}
