#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/inet.h>

MODULE_AUTHOR("Yan Novikov");
MODULE_DESCRIPTION("IP blacklist firewall Eltex School LR6");
MODULE_LICENSE("GPL");

#define MAX_ENTRIES 5

static struct nf_hook_ops nf_in;
static struct kobject *blacklist_kobj;
static char blacklist_ip[MAX_ENTRIES][16] = {0};

// На основе hook из примера
static unsigned int hook_func_in(void *priv, struct sk_buff *skb,
                                 const struct nf_hook_state *state)
{
    struct iphdr *ip_header;
    char src_ip[16];
    int i;

    ip_header = (struct iphdr *)skb_network_header(skb);
    snprintf(src_ip, sizeof(src_ip), "%pI4", &ip_header->saddr); // сохраняем адрес отправителя в src_ip
    // Цикл для отбрасывания пакетов с ip из blacklist
    for (i = 0; i < MAX_ENTRIES; i++) {
        if (strcmp(blacklist_ip[i], src_ip) == 0) {
            printk(KERN_INFO "blacklist: dropped packet from %s\n", src_ip);
            return NF_DROP;
        }
    }
    return NF_ACCEPT;
}

// Функция на основе foo_show
static ssize_t ip_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int index = attr->attr.name[2] - '0';   // получение id из имени файла
    return sprintf(buf, "%s\n", blacklist_ip[index]);
}

// Функция на основе foo_store
static ssize_t ip_store(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf, size_t count)
{
    int index = attr->attr.name[2] - '0';
    sscanf(buf, "%15s", blacklist_ip[index]); // копирование из буфера в элемент массива
    return count;
}

// инициализация атрибутов для каждого ip из массива
static struct kobj_attribute ip0_attr = __ATTR(ip0, 0660, ip_show, ip_store);
static struct kobj_attribute ip1_attr = __ATTR(ip1, 0660, ip_show, ip_store);
static struct kobj_attribute ip2_attr = __ATTR(ip2, 0660, ip_show, ip_store);
static struct kobj_attribute ip3_attr = __ATTR(ip3, 0660, ip_show, ip_store);
static struct kobj_attribute ip4_attr = __ATTR(ip4, 0660, ip_show, ip_store);

// Две структуры ниже - простой способ создать группу однотипных файлов (с нашими ip)
static struct attribute *blacklist_attrs[] = {
    &ip0_attr.attr,
    &ip1_attr.attr,
    &ip2_attr.attr,
    &ip3_attr.attr,
    &ip4_attr.attr,
    NULL,
};

static struct attribute_group blacklist_attr_group = {
    .attrs = blacklist_attrs,
};

static int __init blacklist_init(void)
{
    int error;

    // Настройка netfilter из примера
    nf_in.hook     = hook_func_in;
    nf_in.hooknum  = NF_INET_PRE_ROUTING;
    nf_in.pf       = PF_INET;
    nf_in.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nf_in);

    // Создание sysfs из примера ЛР3
    blacklist_kobj = kobject_create_and_add("blacklist", kernel_kobj);
    if (!blacklist_kobj) {
        nf_unregister_net_hook(&init_net, &nf_in);
        return -ENOMEM;
    }
    // Создание группы однотипных файлов
    error = sysfs_create_group(blacklist_kobj, &blacklist_attr_group);
    if (error) {
        kobject_put(blacklist_kobj);
        nf_unregister_net_hook(&init_net, &nf_in);
        return error;
    }

    printk(KERN_INFO "blacklist: module loaded\n");
    return 0;
}

static void __exit blacklist_exit(void)
{
    nf_unregister_net_hook(&init_net, &nf_in);
    sysfs_remove_group(blacklist_kobj, &blacklist_attr_group);
    kobject_put(blacklist_kobj);
    printk(KERN_INFO "blacklist: module unloaded\n");
}

module_init(blacklist_init);
module_exit(blacklist_exit);