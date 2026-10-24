// Common.glsl is required. iChannel0 = Buffer A.
vec3 scene(vec3 p) {
    vec3 hit = worldScene(p);
    if (control.z > 0.5)
        hit = nearer(hit, vec3(length(p - shot.xyz) - 0.1, 4.0, 0.0));
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
    loadState();

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
        vec3 hit = scene(p);
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
