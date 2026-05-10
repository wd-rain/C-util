<template>
  <div class="about">
    <h1>About</h1>
    <section class="about-content">
      <h2>WDLib Blog</h2>
      <p>A personal blog for sharing embedded development notes, C programming tips, and project experiences.</p>

      <h2>Author</h2>
      <p>An embedded software developer passionate about writing clean, portable C code for resource-constrained systems.</p>

      <h2>Tech Stack</h2>
      <ul>
        <li>Vue 3 + Composition API</li>
        <li>Vue Router (Hash mode)</li>
        <li>Vite</li>
        <li>GitHub Pages</li>
      </ul>

      <h2>Contact</h2>
      <p>GitHub: <a href="https://github.com" target="_blank">My GitHub</a></p>
    </section>
  </div>
</template>

<script setup>
</script>

<style scoped>
.about {
  max-width: 720px;
  margin: 0 auto;
  padding: 2rem 0;
}

.about > h1 {
  font-size: 1.8rem;
  color: #1a1a2e;
  margin-bottom: 2rem;
}

.about-content h2 {
  font-size: 1.2rem;
  color: #1a1a2e;
  margin: 1.5rem 0 0.6rem;
}

.about-content p {
  color: #555;
  line-height: 1.7;
}

.about-content ul {
  padding-left: 1.5rem;
  color: #555;
}

.about-content li {
  margin-bottom: 0.3rem;
}

.about-content a {
  color: #0a7ea4;
  text-decoration: none;
}

.about-content a:hover {
  text-decoration: underline;
}
</style>
