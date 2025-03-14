## 2D SPH fluid simulation

### What is this
This is my math modelling university project. For a long time I've wanted to explore the subject of fluid 
simulations, especially real-time.

### Motivation
I see zero to none usage of water physics in video games, either because it is too resource-heavy or too 
hard to implement.

Or perhaps because nobody cares :<

I'm eager to write an extremely optimized implementation of SPH and use it in my sandbox indie game, with this 
project being the first step.

### What's done (and what's to come)
- [x] Physics simulation using SPH algorithm
- [x] Rendering (using SFML)
- [ ] Parallel physics calculation
- [ ] Parallel rendering
- [ ] Use GPU (Cuda/Compute Shaders/P)
- [ ] Reduce the checks of neighbours for each particle (use Uniform Grid or KD-Tree)
- [ ] Write a full game engine and add this as a module :D