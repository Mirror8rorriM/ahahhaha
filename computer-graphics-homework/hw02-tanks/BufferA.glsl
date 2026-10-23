// iChannel0 = Buffer A (feedback), iChannel1 = Keyboard.
const float PI = 3.14159265359;

float key(int code, int row) {
    return texelFetch(iChannel1, ivec2(code, row), 0).r;
}

float sdBox(vec3 p, vec3 size) {
    vec3 d = abs(p) - size;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

float targetDistance(vec3 p) {
    p -= vec3(3.0, 0.0, 0.0);
    float hull = sdBox(p - vec3(0.0, 0.63, 0.0), vec3(1.12, 0.43, 0.72));
    float turret = sdBox(p - vec3(0.0, 1.27, 0.0), vec3(0.65, 0.30, 0.53));
    return min(hull, turret);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    ivec2 pixel = ivec2(fragCoord);
    fragColor = vec4(0.0);
    if (pixel.y != 0 || pixel.x > 2) return;

    // Texel 0: yaw, elevation, projectile-active, unused.
    // Texel 1: projectile position xyz, age.
    // Texel 2: projectile velocity xyz, unused.
    vec4 control = texelFetch(iChannel0, ivec2(0, 0), 0);
    vec4 shot = texelFetch(iChannel0, ivec2(1, 0), 0);
    vec4 velocity = texelFetch(iChannel0, ivec2(2, 0), 0);

    if (iFrame == 0) {
        control = vec4(0.0, radians(12.0), 0.0, 0.0);
        shot = vec4(0.0);
        velocity = vec4(0.0);
    } else {
        float dt = clamp(iTimeDelta, 0.0, 0.05);
        control.x += (key(37, 0) - key(39, 0)) * dt * 0.85;
        control.x = mod(control.x + PI, 2.0 * PI) - PI;
        control.y = clamp(control.y + (key(38, 0) - key(40, 0)) * dt * 0.5,
                          radians(2.0), radians(65.0));

        if (control.z > 0.5) {
            // Small steps prevent a fast projectile skipping the target.
            for (int j = 0; j < 8; ++j) {
                if (control.z < 0.5) break;
                float h = dt / 8.0;
                shot.xyz += velocity.xyz * h + vec3(0.0, -0.5 * 9.81 * h * h, 0.0);
                velocity.y -= 9.81 * h;
                shot.w += h;
                if (shot.y <= 0.1 || targetDistance(shot.xyz) <= 0.1 || shot.w > 8.0)
                    control.z = 0.0;
            }
        }

        // Row 1 is a press pulse: holding Space does not repeat the shot.
        if (key(32, 1) > 0.5 && control.z < 0.5) {
            vec3 direction = vec3(cos(control.y) * cos(control.x),
                                  sin(control.y), -cos(control.y) * sin(control.x));
            shot = vec4(vec3(-3.0, 1.37, 0.0) + direction * 1.85, 0.0);
            velocity = vec4(direction * 9.0, 0.0);
            control.z = 1.0;
        }
    }

    if (pixel.x == 0) fragColor = control;
    if (pixel.x == 1) fragColor = shot;
    if (pixel.x == 2) fragColor = velocity;
}
