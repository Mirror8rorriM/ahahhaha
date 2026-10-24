// Shared by Buffer A and Image. Channels: iChannel0 = Buffer A.
const float PI = 3.14159265359;
const int MAX_CRATERS = 64;
vec4 control, shot;
int craterCount;
vec4 craters[MAX_CRATERS]; // xyz: local sphere center; w: owning part ID.

void loadState() {
    control = texelFetch(iChannel0, ivec2(0, 0), 0);
    shot = texelFetch(iChannel0, ivec2(1, 0), 0);
    craterCount = clamp(int(texelFetch(iChannel0, ivec2(3, 0), 0).x), 0, MAX_CRATERS);
    for (int i = 0; i < MAX_CRATERS; ++i) {
        if (i >= craterCount) break;
        craters[i] = texelFetch(iChannel0, ivec2(4 + i, 0), 0);
    }
}

vec4 axisAngle(vec3 axis, float angle) {
    return vec4(normalize(axis) * sin(angle * 0.5), cos(angle * 0.5));
}
vec4 qMul(vec4 a, vec4 b) {
    return vec4(a.w * b.xyz + b.w * a.xyz + cross(a.xyz, b.xyz),
                a.w * b.w - dot(a.xyz, b.xyz));
}
vec3 toLocal(vec4 q, vec3 p) {
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
vec3 nearer(vec3 a, vec3 b) { return a.x < b.x ? a : b; }

// Part IDs: ground=1; player hull/tracks=2, turret=3, barrel=4;
// target hull/tracks=5, turret=6, barrel=7.
vec3 partLocal(vec3 p, int part) {
    if (part == 1) return p;
    bool player = part <= 4;
    p -= player ? vec3(-3.0, 0.0, 0.0) : vec3(3.0, 0.0, 0.0);
    int detail = part - (player ? 2 : 5);
    if (detail == 0) return p;
    vec4 yaw = axisAngle(vec3(0.0, 1.0, 0.0), player ? control.x : PI);
    if (detail == 1) return toLocal(yaw, p - vec3(0.0, 1.27, 0.0));
    vec4 rotation = qMul(yaw, axisAngle(vec3(0.0, 0.0, 1.0),
                                      player ? control.y : radians(8.0)));
    return toLocal(rotation, p - vec3(0.0, 1.37, 0.0));
}

float carve(vec3 localPoint, float distanceToPart, int part) {
    float radius = part == 1 ? 0.8 : 0.35;
    for (int i = 0; i < MAX_CRATERS; ++i) {
        if (i >= craterCount) break;
        if (int(craters[i].w) != part) continue;
        float sphere = length(localPoint - craters[i].xyz) - radius;
        distanceToPart = max(distanceToPart, -sphere);
    }
    return distanceToPart;
}

// Return (distance, material ID, part ID). Projectile is excluded from physics.
vec3 tank(vec3 p, bool player) {
    int base = player ? 2 : 5;
    float color = player ? 1.0 : 2.0;
    vec3 local = partLocal(p, base);
    vec3 result = vec3(sdBox(local - vec3(0.0, 0.63, 0.0), vec3(1.12, 0.43, 0.72)), color, float(base));
    for (int side = -1; side <= 1; side += 2) {
        vec3 track = local - vec3(0.0, 0.34, float(side) * 0.78);
        result = nearer(result, vec3(sdBox(track, vec3(1.20, 0.28, 0.19)), 3.0, float(base)));
    }
    result.x = carve(local, result.x, base);
    local = partLocal(p, base + 1);
    float turret = carve(local, sdBox(local, vec3(0.65, 0.30, 0.53)), base + 1);
    result = nearer(result, vec3(turret, color, float(base + 1)));
    local = partLocal(p, base + 2);
    float barrel = carve(local, cylinderX(local - vec3(1.05, 0.0, 0.0), 0.79, 0.09), base + 2);
    return nearer(result, vec3(barrel, 3.0, float(base + 2)));
}
vec3 worldScene(vec3 p) {
    vec3 hit = vec3(carve(p, p.y, 1), 0.0, 1.0);
    hit = nearer(hit, tank(p, true));
    return nearer(hit, tank(p, false));
}
