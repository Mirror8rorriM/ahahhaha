// Buffer A: iChannel0 = Buffer A (feedback), iChannel1 = Keyboard.
// State occupies four texels on the bottom row. No external assets.
const float PI = 3.14159265359;
const vec3 BASE = vec3(-3.0, 0.0, 0.0);
const vec3 TARGET = vec3(3.0, 0.0, 0.0);
float key(int code) { return texelFetch(iChannel1, ivec2(code, 0), 0).r; }
float pressed(int code) { return texelFetch(iChannel1, ivec2(code, 1), 0).r; }
vec4 state(int x) { return texelFetch(iChannel0, ivec2(x, 0), 0); }
float boxDistance(vec3 p, vec3 b) {
    vec3 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}
float targetDistance(vec3 p) {
    p -= TARGET;
    float hull = boxDistance(p - vec3(0, .63, 0), vec3(1.12, .43, .72));
    float turret = boxDistance(p - vec3(0, 1.27, 0), vec3(.65, .30, .53));
    return min(hull, turret);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    ivec2 pixel = ivec2(fragCoord);
    fragColor = vec4(0);
    if (pixel.y != 0 || pixel.x > 3) return;
    // 0: yaw, elevation, projectile-active, unused
    // 1: projectile position, age
    // 2: projectile velocity, unused
    // 3: explosion position, remaining lifetime
    vec4 control = state(0), shot = state(1), velocity = state(2), blast = state(3);
    if (iFrame == 0 || key(82) > .5) {
        control = vec4(0, radians(12.0), 0, 0);
        shot = vec4(0); velocity = vec4(0); blast = vec4(0);
    } else {
        float dt = clamp(iTimeDelta, 0.0, .05);
        control.x += (key(37) + key(65) - key(39) - key(68)) * dt * .85;
        control.x = mod(control.x + PI, 2.0 * PI) - PI;
        control.y = clamp(control.y + (key(38) + key(87) - key(40) - key(83)) * dt * .5,
                          radians(2.0), radians(65.0));
        blast.w = max(0.0, blast.w - dt);
        if (control.z > .5) {
            // Substeps avoid tunnelling through the target at low frame rates.
            for (int j = 0; j < 8; ++j) {
                if (control.z < .5) break;
                float h = dt / 8.0;
                shot.xyz += velocity.xyz * h + vec3(0, -.5 * 9.81 * h * h, 0);
                velocity.y -= 9.81 * h;
                shot.w += h;
                if (shot.y < .09 || targetDistance(shot.xyz) < .10) {
                    blast = vec4(shot.xyz, 1.25);
                    control.z = 0.0;
                }
                if (shot.w > 8.0 || length(shot.xyz) > 65.0) control.z = 0.0;
            }
        }
        // Keyboard row 1 is a press pulse, so holding Space fires only once.
        if (pressed(32) > .5 && control.z < .5) {
            vec3 direction = vec3(cos(control.y) * cos(control.x), sin(control.y),
                                  -cos(control.y) * sin(control.x));
            shot = vec4(BASE + vec3(0, 1.37, 0) + direction * 1.85, 0);
            velocity = vec4(direction * 9.0, 0);
            control.z = 1.0;
        }
    }
    if (pixel.x == 0) fragColor = control;
    if (pixel.x == 1) fragColor = shot;
    if (pixel.x == 2) fragColor = velocity;
    if (pixel.x == 3) fragColor = blast;
}
