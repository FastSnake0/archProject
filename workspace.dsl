workspace {
    name "SocArt"
    description "Простая социальная сеть"

    # включаем режим с иерархической системой идентификаторов
    !identifiers hierarchical

    # Модель архитектуры
    model {

        # Настраиваем возможность создания вложенных груп
        properties { 
            structurizr.groupSeparator "/"
        }
        

        # Описание компонент модели
        user = person "Пользователь"
        social = softwareSystem "Соцсеть" {
            description "Сервис социальной сети"

            gateway_service = container "Gateway service" {
                description "Сервис для перенаправления запросов"
            }

            user_service = container "User service" {
                description "Сервис управления пользователями"
            }

            post_service = container "Post service" {
                description "Сервис управления лентой"
            }

            chat_service = container "Сhat service" {
                description "Сервис управления чатом"
            }

            group "Слой данных" {
                user_database = container "User Database" {
                    description "База данных с пользователями"
                    technology "PostgreSQL 15"
                    tags "database"
                }

                user_cache = container "User Cache" {
                    description "Кеш пользовательских данных для ускорения аутентификации"
                    technology "Redis:6.2-alpine"
                    tags "database"
                }

                content_database = container "Content Database" {
                    description "База данных для хранения ленты и личных сообщений"
                    technology "MongoDB 5"
                    tags "database"
                }

            }

            

            user -> gateway_service "Просмотр ленты и отправка сообщений. Поиск других пользователей"

            gateway_service -> user_service "API запрос от пользователя в сервис пользователей"
            gateway_service -> post_service "API запрос от пользователя в сервис постов"
            gateway_service -> chat_service "API запрос от пользователя в сервис чата"
            gateway_service -> user_cache "Получение кэшированных данных"

            user_service -> user_database "Получение/обновление данных о пользователях"
            post_service -> content_database "Получение/обновление данных о ленте"
            chat_service -> content_database "Получение/обновление данных о чате"
        }
        #1

        deploymentEnvironment "Deploy" {

            deploymentNode "Gateway Server" {
                containerInstance social.gateway_service
            }


            deploymentNode "User Server" {
                containerInstance social.user_service
            }

            deploymentNode "Post Server" {
                containerInstance social.post_service
            }

            deploymentNode "Chat Server" {
                containerInstance social.chat_service
            }

            deploymentNode "databases" {
     
                deploymentNode "Database Server 1" {
                    containerInstance social.user_database
                }

                deploymentNode "Database Server 2" {
                    containerInstance social.content_database
                    instances 3
                }

                deploymentNode "Cache Server" {
                    containerInstance social.user_cache
                }
            }
            
        }
    }

    views {
        themes default

        properties { 
            structurizr.tooltips true
        }
        /*
        !script groovy {
            workspace.views.createDefaultViews()
            workspace.views.views.findAll { it instanceof com.structurizr.view.ModelView }.each { it.enableAutomaticLayout() }
        }*/

        
        systemContext social {
            autoLayout
            include *
        }

        container social {
            autoLayout
            include *
        }

        deployment social "Deploy" "deployment" {
             include *
             autoLayout
        }
        

        dynamic social "UC01" "Добавление нового пользователя" {
            autoLayout
            user -> social.gateway_service "Создать нового пользователя (логин) (POST /user)"
            social.gateway_service -> social.user_service "Если логин не занят" 
            social.user_service -> social.user_database "Сохранить данные о пользователе"
        }

        dynamic social "UC02" "Поиск пользователя по логину" {
            autoLayout
            user -> social.gateway_service "Найти пользователя по логину ({login}) (GET /user)"
            social.gateway_service -> social.user_service "Auth"
            social.user_service -> social.gateway_service "JWT token"
            social.gateway_service -> social.user_cache "Найти пользователя в кэше"
            social.gateway_service -> social.user_service "Запрос Найти пользователя в бд, если нет в кэше"
            social.user_service -> social.user_database "Найти пользователя в бд, если нет в кэше"
            
        }

        dynamic social "UC03" "Поиск пользователя по маске имя и фамилии" {
            autoLayout
            user -> social.gateway_service "Найти пользователя по маске ({mask}) (GET /user)"

            social.gateway_service -> social.user_service "Auth"
            social.user_service -> social.gateway_service "JWT token"

            social.gateway_service -> social.user_cache "Найти пользователя в кэше"
            social.gateway_service -> social.user_service "Запрос Найти пользователя в бд, если нет в кэше"
            social.user_service -> social.user_database "Найти пользователя в бд, если нет в кэше"
        }

        dynamic social "UC11" "Добавление записи на стену" {
            autoLayout
            user -> social.gateway_service "Создать пост (POST /user/post)"

            social.gateway_service -> social.user_service "Auth"
            social.user_service -> social.user_database
            social.user_database -> social.user_service
            social.user_service -> social.gateway_service "JWT token"

            social.gateway_service -> social.post_service
            social.post_service -> social.content_database "Сохранить пост"

        }

        dynamic social "UC12" "Загрузка стены пользователя" {
            autoLayout
            user -> social.gateway_service "Загрузка постов пользователя (GET /user/posts)"

            social.gateway_service -> social.user_service "Auth"
            social.user_service -> social.user_database
            social.user_database -> social.user_service
            social.user_service -> social.gateway_service "JWT token"

            social.gateway_service -> social.post_service "Запрос Загрузить стену пользователя"
            social.post_service -> social.content_database "Загрузить стену пользователя"
        }

        dynamic social "UC21" "Отправка сообщения пользователю" {
            autoLayout
            user -> social.gateway_service "Отправить сообщение (POST /user/message)"

            social.gateway_service -> social.user_service "Auth"
            social.user_service -> social.user_database
            social.user_database -> social.user_service
            social.user_service -> social.gateway_service "JWT token"

            social.gateway_service -> social.chat_service "Запрос Отправить сообщение"
            social.chat_service -> social.content_database "Отправить сообщение"
        }

        dynamic social "UC22" "Получение списка сообщения для пользователя" {
            autoLayout
            user -> social.gateway_service "Отправить сообщение (POST /user/chat)"

            social.gateway_service -> social.user_service "Auth"
            social.user_service -> social.user_database
            social.user_database -> social.user_service
            social.user_service -> social.gateway_service "JWT token"

            social.gateway_service -> social.chat_service "Запрос Загрузить Сообщения для пользователя"
            social.chat_service -> social.content_database "Загрузить Сообщения для пользователя"
        }


    


        styles {
            element "database" {
                shape cylinder
            }
        }
    }
}