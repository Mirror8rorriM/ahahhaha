// iChannel0 = Buffer A. Quaternion layout: (x, y, z, w).
const float PI = 3.14159265359;
vec4 control, shot;

vec4 axisAngle(vec3 axis, float angle) {
    return vec4(normalize(axis) * sin(angle * 0.5), cos(angle * 0.5));
}

vec4 qMul(vec4 a, vec4 b) {
    return vec4(a.w * b.xyz + b.w * a.xyz + cross(a.xyz, b.xyz),
                a.w * b.w - dot(a.xyz, b.xyz));
}

vec3 toLocal(vec4 q, vec3 p) {
    // Apply the inverse rotation: conjugate(q) * (p, 0) * q.
    vec3 v = -q.xyz;
    return p + 2.0 * cross(v, cross(v, p) + q.w * p);
}

float sdBox(vec3 p, vec3 size) {
    vec3 d = abs(p) - size;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

float cylinderX(vec3 p, float halfLength, float radius) {
    vec2 d = vec2(abs(p.x) - halfLength, length(p.yz) - radius);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

vec2 nearer(vec2 a, vec2 b) {
    return a.x < b.x ? a : b;
}

vec2 tank(vec3 p, vec3 center, float yaw, float elevation, float colorId) {
    p -= center;
    vec2 result = vec2(sdBox(p - vec3(0.0, 0.63, 0.0), vec3(1.12, 0.43, 0.72)), colorId);
    for (int side = -1; side <= 1; side += 2) {
        vec3 track = p - vec3(0.0, 0.34, float(side) * 0.78);
        result = nearer(result, vec2(sdBox(track, vec3(1.20, 0.28, 0.19)), 3.0));
    }

    vec4 yawQ = axisAngle(vec3(0.0, 1.0, 0.0), yaw);
    vec3 turret = toLocal(yawQ, p - vec3(0.0, 1.27, 0.0));
    result = nearer(result, vec2(sdBox(turret, vec3(0.65, 0.30, 0.53)), colorId));

    vec4 barrelQ = qMul(yawQ, axisAngle(vec3(0.0, 0.0, 1.0), elevation));
    vec3 barrel = toLocal(barrelQ, p - vec3(0.0, 1.37, 0.0));
    return nearer(result, vec2(cylinderX(barrel - vec3(1.05, 0.0, 0.0), 0.79, 0.09), 3.0));
}

vec2 scene(vec3 p) {
    vec2 hit = vec2(p.y, 0.0);
    hit = nearer(hit, tank(p, vec3(-3.0, 0.0, 0.0), control.x, control.y, 1.0));
    hit = nearer(hit, tank(p, vec3(3.0, 0.0, 0.0), PI, radians(8.0), 2.0));
    if (control.z > 0.5)
        hit = nearer(hit, vec2(length(p - shot.xyz) - 0.1, 4.0));
    return hit;
}

vec3 normalAt(vec3 p) {
    vec2 e = vec2(0.001, 0.0);
    return normalize(vec3(scene(p + e.xyy).x - scene(p - e.xyy).x,
                          scene(p + e.yxy).x - scene(p - e.yxy).x,
                          scene(p + e.yyx).x - scene(p - e.yyx).x));
}

vec3 material(float id) {
    if (id < 0.5) return vec3(0.35, 0.30, 0.20);
    if (id < 1.5) return vec3(0.20, 0.45, 0.20);
    if (id < 2.5) return vec3(0.55, 0.25, 0.15);
    if (id < 3.5) return vec3(0.12);
    return vec3(1.0, 0.8, 0.1);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    control = texelFetch(iChannel0, ivec2(0, 0), 0);
    shot = texelFetch(iChannel0, ivec2(1, 0), 0);

    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec3 camera = vec3(8.0, 6.0, 10.0);
    vec3 forward = normalize(vec3(0.0, 0.65, 0.0) - camera);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);
    vec3 ray = mat3(right, up, forward) * normalize(vec3(uv * tan(radians(48.0) * 0.5), 1.0));

    vec3 color = vec3(0.6, 0.75, 0.9);
    float distanceAlongRay = 0.0;
    for (int j = 0; j < 150; ++j) {
        vec3 p = camera + ray * distanceAlongRay;
        vec2 hit = scene(p);
        if (hit.x < 0.0015) {
            vec3 light = normalize(vec3(-0.6, 1.0, 0.5));
            float diffuse = max(dot(normalAt(p), light), 0.0);
            color = material(hit.y) * (0.3 + 0.7 * diffuse);
            break;
        }
        distanceAlongRay += hit.x;
        if (distanceAlongRay > 60.0) break;
    }
    fragColor = vec4(color, 1.0);
}
