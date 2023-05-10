export default function setActiveNavLink() {
    const currentUrl = window.location.pathname;
    let links = document.querySelectorAll('nav a');
    for (let i = 0; i < links.length; i++) {
        if (links[i].classList.contains('navbar-brand'))
            continue;
        const linkUrl = links[i].pathname;
        if (linkUrl === '/' && currentUrl === '/') {
            links[i].classList.add('active');
            break;
        } else if (linkUrl !== '/' && currentUrl.startsWith(linkUrl)) {
            links[i].classList.add('active');
            break;
        }
    }
}