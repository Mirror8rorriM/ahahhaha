// Common.glsl is required. iChannel0 = Buffer A, iChannel1 = Keyboard.
float key(int code, int row) {
    return texelFetch(iChannel1, ivec2(code, row), 0).r;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    ivec2 pixel = ivec2(fragCoord);
    fragColor = vec4(0.0);
    if (pixel.y != 0 || pixel.x >= 4 + MAX_CRATERS) return;
    loadState();
    vec4 velocity = texelFetch(iChannel0, ivec2(2, 0), 0);
    int previousCount = craterCount;
    vec4 newCrater = vec4(0.0);
    if (iFrame == 0) {
        control = vec4(0.0, radians(12.0), 0.0, 0.0);
        shot = vec4(0.0);
        velocity = vec4(0.0);
        craterCount = 0;
    } else {
        float dt = clamp(iTimeDelta, 0.0, 0.05);
        control.x += (key(37, 0) - key(39, 0)) * dt * 0.85;
        control.x = mod(control.x + PI, 2.0 * PI) - PI;
        control.y = clamp(control.y + (key(38, 0) - key(40, 0)) * dt * 0.5,
                          radians(2.0), radians(65.0));
        if (control.z > 0.5) {
            for (int j = 0; j < 8; ++j) {
                if (control.z < 0.5) break;
                float h = dt / 8.0;
                shot.xyz += velocity.xyz * h + vec3(0.0, -0.5 * 9.81 * h * h, 0.0);
                velocity.y -= 9.81 * h;
                shot.w += h;
                vec3 hit = worldScene(shot.xyz);
                if (hit.x <= 0.1) {
                    if (craterCount < MAX_CRATERS) {
                        int part = int(hit.z);
                        vec3 center = partLocal(shot.xyz, part);
                        if (part == 1) center.y += 0.3;
                        newCrater = vec4(center, float(part));
                        craterCount++;
                    }
                    control.z = 0.0;
                } else if (shot.w > 8.0) control.z = 0.0;
            }
        }
        if (key(32, 1) > 0.5 && control.z < 0.5) {
            vec3 direction = vec3(cos(control.y) * cos(control.x),
                                  sin(control.y), -cos(control.y) * sin(control.x));
            shot = vec4(vec3(-3.0, 1.37, 0.0) + direction * 1.96, 0.0);
            velocity = vec4(direction * 9.0, 0.0);
            control.z = 1.0;
        }
    }
    if (pixel.x == 0) fragColor = control;
    if (pixel.x == 1) fragColor = shot;
    if (pixel.x == 2) fragColor = velocity;
    if (pixel.x == 3) fragColor = vec4(float(craterCount), 0.0, 0.0, 0.0);
    if (pixel.x >= 4 && iFrame != 0) {
        fragColor = pixel.x == 4 + previousCount && craterCount > previousCount
                  ? newCrater : texelFetch(iChannel0, pixel, 0);
    }
}
