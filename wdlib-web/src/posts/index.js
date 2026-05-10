export const posts = [
  {
    id: 'hello-world',
    title: 'Hello World - My First Post',
    date: '2026-05-11',
    tags: ['blog'],
    excerpt: 'Welcome to my blog. This is the first post to test the blog system.',
    content: `
      <p>Welcome to WDLib Blog!</p>
      <p>This is a simple blog built with Vue 3 + Vite, hosted on GitHub Pages.</p>
      <p>I will share my experience and notes about embedded development, C programming, and other topics here.</p>
      <h3>Tech Stack</h3>
      <ul>
        <li>Vue 3 + Composition API</li>
        <li>Vue Router</li>
        <li>Vite</li>
        <li>GitHub Pages</li>
      </ul>
      <p>Stay tuned for more posts!</p>
    `
  },
  {
    id: 'about-wdlib',
    title: 'About WDLib - An Embedded C Utility Library',
    date: '2026-05-11',
    tags: ['embedded', 'C'],
    excerpt: 'Introducing WDLib, a lightweight C utility library designed for embedded systems.',
    content: `
      <p>WDLib is a collection of reusable C modules for embedded development.</p>
      <h3>Modules</h3>
      <ul>
        <li><strong>OS Utilities</strong> - Event system, timer management</li>
        <li><strong>Platform Abstraction</strong> - GPIO, I2C, ISR unified interfaces</li>
        <li><strong>Tools</strong> - Ring buffer, command line parser</li>
      </ul>
      <h3>Design Goals</h3>
      <ul>
        <li>Minimal dependencies</li>
        <li>Easy to port across MCU platforms</li>
        <li>Clean C99 code</li>
      </ul>
    `
  }
]
