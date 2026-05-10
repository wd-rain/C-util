import { createRouter, createWebHashHistory } from 'vue-router'
import Home from '../views/Home.vue'
import PostDetail from '../views/PostDetail.vue'
import Archives from '../views/Archives.vue'
import About from '../views/About.vue'

const routes = [
  { path: '/', name: 'Home', component: Home },
  { path: '/post/:id', name: 'Post', component: PostDetail },
  { path: '/archives', name: 'Archives', component: Archives },
  { path: '/about', name: 'About', component: About }
]

const router = createRouter({
  history: createWebHashHistory(),
  routes
})

export default router
