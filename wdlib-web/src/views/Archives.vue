<template>
  <div class="archives">
    <h1>Archives</h1>
    <div class="timeline">
      <div class="year-group" v-for="(group, year) in groupedPosts" :key="year">
        <h2>{{ year }}</h2>
        <ul>
          <li v-for="post in group" :key="post.id">
            <span class="date">{{ post.date }}</span>
            <router-link :to="`/post/${post.id}`">{{ post.title }}</router-link>
          </li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { posts } from '../posts'

const groupedPosts = computed(() => {
  const groups = {}
  posts.forEach(post => {
    const year = post.date.substring(0, 4)
    if (!groups[year]) groups[year] = []
    groups[year].push(post)
  })
  return groups
})
</script>

<style scoped>
.archives {
  max-width: 720px;
  margin: 0 auto;
  padding: 2rem 0;
}

.archives > h1 {
  font-size: 1.8rem;
  color: #1a1a2e;
  margin-bottom: 2rem;
}

.year-group h2 {
  font-size: 1.3rem;
  color: #1a1a2e;
  margin-bottom: 0.8rem;
  padding-bottom: 0.4rem;
  border-bottom: 2px solid #00d2ff;
  display: inline-block;
}

.year-group ul {
  list-style: none;
  padding: 0;
  margin-bottom: 2rem;
}

.year-group li {
  padding: 0.6rem 0;
  display: flex;
  align-items: center;
  gap: 1rem;
}

.date {
  color: #888;
  font-size: 0.85rem;
  min-width: 90px;
}

.year-group a {
  color: #333;
  text-decoration: none;
}

.year-group a:hover {
  color: #0a7ea4;
}
</style>
