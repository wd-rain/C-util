<template>
  <header class="header">
    <div class="header-inner">
      <router-link to="/" class="logo">WDLib</router-link>
      <nav class="nav">
        <router-link to="/">Home</router-link>
        <router-link to="/archives">Archives</router-link>
        <router-link to="/about">About</router-link>
      </nav>
    </div>
  </header>
</template>

<script setup>
</script>

<style scoped>
.header {
  background: #1a1a2e;
  padding: 0 2rem;
  position: sticky;
  top: 0;
  z-index: 100;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
}

.header-inner {
  max-width: 960px;
  margin: 0 auto;
  display: flex;
  align-items: center;
  justify-content: space-between;
  height: 60px;
}

.logo {
  font-size: 1.4rem;
  font-weight: bold;
  color: #00d2ff;
  text-decoration: none;
}

.nav {
  display: flex;
  gap: 1.5rem;
}

.nav a {
  color: #aaa;
  text-decoration: none;
  font-size: 0.95rem;
  transition: color 0.2s;
}

.nav a:hover,
.nav a.router-link-active {
  color: #00d2ff;
}
</style>
