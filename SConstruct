env = Environment(
    CXXFLAGS = ["-std=c++14", "-Wall", "-O3"],
    LIBS = ['SDL2', 'SDL2_image']
)

env.Program(target="fflego", source=env.Glob("*.cpp"))
